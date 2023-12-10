import { Manager } from '../control/manager';
import { Processes } from '../services/processes';
import { LogLevel } from '../util/logger';
import { ServerState } from '../types/monitor';
import { ModUpdatedStatus } from '../types/steamcmd';
import { IStatefulService } from '../types/service';
import { inject, injectable, singleton } from 'tsyringe';
import { LoggerFactory } from './loggerfactory';
import { FSAPI, InjectionTokens } from '../util/apis';
import { EventBus } from '../control/event-bus';
import { InternalEventTypes } from '../types/events';
import { ServerStarter } from './server-starter';
import { ServerDetector } from './server-detector';
import { SteamCMD } from '../services/steamcmd';
import { DiscordEventConverter } from '../services/discord-event-converter';

export type ServerStateListener = (state: ServerState) => any;

@singleton()
@injectable()
export class Monitor extends IStatefulService {

    public loopInterval = 500;
    private tickRunning = false;
    private lastTick = 0;
    private modUpdateCheckerRunning = false;
    private lastModUpdateChecker = 0;

    public isModUpdatedMsgSent = false;
    public restartLock: boolean = false;
    private initialStart: boolean = true;

    // cached path to server lock file
    private lockPath: string;

    // saved for determining server stuck state
    private lastServerUsages: number[] = [];

    public $internalServerState: ServerState = ServerState.STOPPED;

    public constructor(
        loggerFactory: LoggerFactory,
        private manager: Manager,
        private eventBus: EventBus,
        private processes: Processes,
        private serverStarter: ServerStarter,
        private serverDetector: ServerDetector,
        private steamCmd: SteamCMD,
        private discordEventConverter: DiscordEventConverter,
        @inject(InjectionTokens.fs) private fs: FSAPI,
    ) {
        super(loggerFactory.createLogger('Monitor'));
    }

    public get internalServerState(): ServerState {
        return this.$internalServerState;
    }

    public set internalServerState(state: ServerState) {
        if (this.$internalServerState === state) return;

        // prevent intermediate state change
        if (
            state === ServerState.STARTED
            && (
                this.$internalServerState === ServerState.STOPPING
            )
        ) {
            // TODO force resume after this occurs multiple times?
            return;
        }

        const previousState = this.$internalServerState;
        this.$internalServerState = state;
        this.manager.curServerState = state;
        this.eventBus.emit(
            InternalEventTypes.MONITOR_STATE_CHANGE,
            this.$internalServerState,
            previousState,
        );
        
        //discordEventConverter
        this.discordEventConverter.handleServerState(this.$internalServerState,previousState);
    }

    public get serverState(): ServerState {
        return this.internalServerState;
    }

    public async killServer(force?: boolean): Promise<boolean> {
        if (this.internalServerState === ServerState.STARTING || this.serverState === ServerState.STARTED) {
            this.internalServerState = ServerState.STOPPING;
            this.manager.curServerState = ServerState.STOPPING;
        }

        return this.serverStarter.killServer(force);
    }

    public async start(): Promise<void> {
        //tickChekerTimer
        //if (this.timers.getTimer('tick')) return;
        if (!this.timers.getTimer('tick')){
            this.lastTick = 0;
            this.tickRunning = false;
            this.timers.addInterval(
                'tick',
                () => {
                    if (this.tickRunning) {
                        return;
                    }
                    if (
                        (new Date().valueOf() - this.lastTick)
                            > this.manager.config.serverProcessPollIntervall
                    ) {
                        this.tickRunning = true;
                        const cb = (): void => {
                            this.tickRunning = false;

                            // set lsat tick time (might be edited by tick itself)
                            this.lastTick = Math.max(
                                this.lastTick,
                                new Date().valueOf(),
                            );
                        };
                        this.tick().then(cb, cb);
                    }
                },
                this.loopInterval,
            );
        }

        //modUpdateCheckerTimer
        //if (this.timers.getTimer('modUpdateChecker')) return;
        if (!this.timers.getTimer('modUpdateChecker')){
            this.log.log(LogLevel.INFO, '[monitor.start] Create modUpdateChecker timer');
            this.lastModUpdateChecker = 0;
            this.modUpdateCheckerRunning = false;
            this.timers.addInterval(
                'modUpdateChecker',
                () => {
                    if (this.modUpdateCheckerRunning) {
                        return;
                    }
                    if (
                        (new Date().valueOf() - this.lastModUpdateChecker)
                            > this.manager.config.modUpdateChekerIntervall
                    ) {
                        this.modUpdateCheckerRunning = true;
                        const cb = (): void => {
                            this.modUpdateCheckerRunning = false;

                            // set lsat tick time (might be edited by tick itself)
                            this.lastModUpdateChecker = Math.max(
                                this.lastModUpdateChecker,
                                new Date().valueOf(),
                            );
                        };
                        this.ModUpadeChecker().then(cb, cb);
                    }
                },
                this.loopInterval,
            );
        }

        this.log.log(LogLevel.IMPORTANT, 'Starting to watch server');
    }

    public async stop(): Promise<void> {
        if (!this.timers.getTimer('tick')) return;
        this.timers.removeAllTimers();
        this.eventBus.clear(InternalEventTypes.MONITOR_STATE_CHANGE);
        this.log.log(LogLevel.IMPORTANT, 'Stoping to watch server');
    }

    private async ModUpadeChecker(): Promise<void>{
        // if (this.manager.config.disableModUpdate) {
        //     return;
        // }
        try {
            if (!await this.steamCmd.updateAllMods()) {
                throw new Error('Updating Mods failed');
            }
        } catch (e) {
            this.log.log(LogLevel.ERROR, '[monitor.ModUpadeChecker] Error during loop', e);
        }
    }

    private async tick(): Promise<void> {
        if (this.manager.config.disableServerMonitoring) {
            return;
        }

        try {
            let needsRestart = true;

            // User locked the server manually
            if (needsRestart && this.manager.config.lockServerRestart) {
                if (!this.manager.config.disableServerLockLogs) {
                    this.log.log(LogLevel.IMPORTANT, 'Detected server lock in config. Skipping server check');
                }
                needsRestart = false;
            }

            // User locked the server manually via file
            if (needsRestart && this.fs.existsSync(this.lockPath)) {
                if (!this.manager.config.disableServerLockLogs) {
                    this.log.log(LogLevel.IMPORTANT, 'Detected manual server lockfile. Skipping server check');
                }
                needsRestart = false;
            }

            // restart locked
            if (needsRestart && this.restartLock) {
                if (!this.manager.config.disableServerLockLogs) {
                    this.log.log(LogLevel.IMPORTANT, 'Detected server restart lock state. Skipping server check');
                }
                needsRestart = false;
            }

            //this.log.log(LogLevel.INFO, 'Start server check...');
            // server running
            if (await this.serverDetector.isServerRunning()) {
                needsRestart = false;
                this.initialStart = false;
                this.internalServerState = ServerState.STARTED;
                this.manager.curServerState = ServerState.STARTED;
            } else {
                this.internalServerState = ServerState.STOPPED;
                this.manager.curServerState = ServerState.STOPPED;

            }
            if(this.serverState !== this.internalServerState) {
                this.log.log(LogLevel.IMPORTANT, 'Server state has changed: '+this.internalServerState);
            }

            if (needsRestart) {
                this.restartServer();
            } else if (this.steamCmd.isNewModUpdated){
                this.log.log(LogLevel.IMPORTANT, `[Monitor.restartServer] isNewModUpdated = ${this.steamCmd.isNewModUpdated}`);
                
                const modUpdateStatus: ModUpdatedStatus = { 
                    modIds: this.steamCmd.listModUpdatedID, 
                    success: this.steamCmd.isNewModUpdated
                };

                if(!this.isModUpdatedMsgSent){
                    this.discordEventConverter.handleModUpdated(modUpdateStatus);
                    this.isModUpdatedMsgSent = true;
                    // testing xoá sau khi test
                    this.steamCmd.isNewModUpdated = false;
                }

                //this.restartServer();
            } else if (!this.manager.config.disableStuckCheck) {
                await this.checkPossibleStuckState();
            } 
        } catch (e) {
            this.log.log(LogLevel.ERROR, 'Error during server monitor loop', e);
        }
    }

    public async restartServer(){
        this.log.log(LogLevel.IMPORTANT, '[Monitor.restartServer] Restarting...');
        this.internalServerState = ServerState.STARTING;
        this.manager.curServerState = ServerState.STARTING;
        await this.serverStarter.killServer();
        await this.serverStarter.startServer(this.initialStart);
        this.lastServerUsages = [];
        this.initialStart = false;
        this.steamCmd.isNewModUpdated = false;

        // give the server a minute to start up
        this.skipLoop(120000);        
    }

    public async checkPossibleStuckState(): Promise<boolean> {
        const processes = await this.serverDetector.getDayZProcesses();

        // no processes found, also means no process to be stuck
        if (!processes?.length) {
            this.lastServerUsages = [];
            return false;
        }

        if (this.lastServerUsages.length >= 5) {
            this.lastServerUsages.shift();
        }
        this.lastServerUsages.push(
            this.processes.getProcessCPUSpent(processes[0]),
        );

        if (this.lastServerUsages.length >= 5) {
            let avg = 0;
            for (const usage of this.lastServerUsages) {
                avg += usage;
            }
            avg /= this.lastServerUsages.length;
            this.log.log(LogLevel.DEBUG, `Dertermined server usage (avg, last 5): ${avg}, [${this.lastServerUsages.join(', ')}]`);
            // if the process spends very little cpu time, it probably got stuck
            if (this.lastServerUsages.every((x) => (Math.abs(avg - x) < 3))) {
                const msg = 'WARNING: Server possibly got stuck!';
                this.log.log(LogLevel.WARN, msg);
                this.eventBus.emit(
                    InternalEventTypes.DISCORD_MESSAGE,
                    {
                        type: 'admin',
                        message: msg,
                    },
                );
                return true;
            }
        }

        return false;
    }

    public skipLoop(forTime?: number): void {
        this.lastTick = new Date().valueOf() + (forTime ?? 30000);
    }

}

