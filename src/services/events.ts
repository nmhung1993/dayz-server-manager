import { IStatefulService } from '../types/service';
import { Manager } from '../control/manager';
import * as cron from 'node-schedule';
import { LogLevel } from '../util/logger';
import { ServerState } from '../types/monitor';
import { injectable, singleton } from 'tsyringe';
import { LoggerFactory } from './loggerfactory';
import { RCON } from './rcon';
import { Monitor } from './monitor';
import { Backups } from './backups';
import { EventBus } from '../control/event-bus';
import { InternalEventTypes } from '../types/events';
import { ServerDetector } from './server-detector';


@singleton()
@injectable()
export class Events extends IStatefulService {

    public tasks: cron.Job[] = [];

    public constructor(
        logerFactory: LoggerFactory,
        private manager: Manager,
        private serverDetector: ServerDetector,
        private monitor: Monitor,
        private rcon: RCON,
        private backup: Backups,
        private eventBus: EventBus,
    ) {
        super(logerFactory.createLogger('Events'));
    }

    public async start(): Promise<void> {

        await this.serverDetector.isServerRunning

        for (const event of (this.manager.config.events ?? [])) {

            const runTask = async (task: () => Promise<any>): Promise<any> => {
                void task()
                    ?.then(() => {
                        this.log.log(LogLevel.DEBUG, `Successfully executed task '${event.name}'`);
                    })
                    ?.catch(/* istanbul ignore next */ () => {
                        this.log.log(LogLevel.WARN, `Failed to execute task '${event.name}'`);
                    });
            };

            const checkAndRun = async (task: () => Promise<any>): Promise<void> => {
                if (this.monitor.serverState !== ServerState.STARTED) {
                    this.log.log(LogLevel.WARN, `Skipping '${event.name}' because server is not in STARTED state. Current state: ${this.monitor.serverState}`);
                    return;
                }

                void runTask(task);
            };

            const executeTime = new Date(this.manager.processCreatedDate.getTime() + event.time*1000*60);
            //this.log.log(LogLevel.INFO, `eventTime = '${event.time}'. executeTime = ${executeTime}`);

            const job = cron.scheduleJob(
                event.name,
                event.time > 0 ? executeTime : event.cron,
                () => {
                    this.log.log(LogLevel.DEBUG, `Executing task '${event.name}' (${event.type})`);
                    switch (event.type) {
                        case 'restart': {
                            void checkAndRun(async () => {
                                this.eventBus.emit(
                                    InternalEventTypes.DISCORD_MESSAGE,
                                    {
                                        type: 'notification',
                                        message: 'Restart server định kỳ!',
                                    },
                                );
                                await this.monitor.killServer();
                            });
                            break;
                        }
                        case 'message': {
                            void checkAndRun(() => this.rcon.global(event.params[0]));
                            break;
                        }
                        case 'kickAll': {
                            void checkAndRun(() => void this.rcon.kickAll());
                            break;
                        }
                        case 'lock': {
                            void checkAndRun(() => void this.rcon.lock());
                            break;
                        }
                        case 'unlock': {
                            void checkAndRun(() => void this.rcon.unlock());
                            break;
                        }
                        case 'backup': {
                            void runTask(() => this.backup.createBackup());
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                },
            );

            this.log.log(
                LogLevel.INFO,
                `Scheduled '${event.name}' with pattern: ${event.cron} (Next run: ${job.nextInvocation().toLocaleString('VN', { timeZone: 'Asia/ho_chi_minh' })})`,
            );


            // tạm tắt push job
            
            this.tasks.push(job);
        }
    }

    public async stop(): Promise<void> {
        for (const task of this.tasks) {
            try {
                task.cancel();
            } catch (e) {
                this.log.log(LogLevel.DEBUG, `Stopping event schedule for '${task.name}' failed`, e);
            }
        }
        this.tasks = [];
    }

}
