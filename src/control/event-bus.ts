import { injectable, singleton } from 'tsyringe';
import { IService } from '../types/service';
import { LoggerFactory } from '../services/loggerfactory';
import { MessageEmbed } from "discord.js";
import { EventEmitter2, Listener } from 'eventemitter2';
import { InternalEventTypes } from '../types/events';
import { CommandMap, Request, Response } from '../types/interface';
import { ServerState } from '../types/monitor';
import { LogEntryEvent } from '../types/log-reader';
import { MetricEntryEvent } from '../types/metrics';
import { DiscordMessage } from '../types/discord';
import { GameUpdatedStatus, ModUpdatedStatus } from '../types/steamcmd';
import { LogLevel } from '../util/logger';

@singleton()
@injectable()
export class EventBus extends IService {

    private readonly EVENT_EMITTER = new EventEmitter2();

    public constructor(
        loggerfactory: LoggerFactory,
    ) {
        super(loggerfactory.createLogger('EventBus'));
    }

    public request(name: InternalEventTypes, data?: any): Promise<Promise<any>[]> {
        return (this.EVENT_EMITTER.emitAsync(name, data));
    }

    public emit(name: InternalEventTypes.DISCORD_MESSAGE, message: DiscordMessage): void;
    public emit(name: InternalEventTypes.DISCORD_MESSAGE, embeds: MessageEmbed): void;
    public emit(name: InternalEventTypes.MONITOR_STATE_CHANGE, newState: ServerState, previousState: ServerState): void;
    public emit(name: InternalEventTypes.METRIC_ENTRY, metricEntryEvent: MetricEntryEvent): void;
    public emit(name: InternalEventTypes.LOG_ENTRY, logEntryEvent: LogEntryEvent): void;
    public emit(name: InternalEventTypes.MOD_UPDATED, status: ModUpdatedStatus): void;
    public emit(name: InternalEventTypes.GAME_UPDATED, status: GameUpdatedStatus): void;
    public emit(
        name: InternalEventTypes.INTERNAL_MOD_INSTALL
        | InternalEventTypes.GET_INTERNAL_MODS,
        data: any,
    ): void;
    public emit(name: InternalEventTypes, data?: any): void {
        try{
            if(name === InternalEventTypes.DISCORD_MESSAGE 
                || name === InternalEventTypes.MONITOR_STATE_CHANGE 
                || name === InternalEventTypes.MOD_UPDATED 
                || name === InternalEventTypes.GAME_UPDATED){
                (data: DiscordMessage) => {
                    const dataLog = data;
                    this?.log?.log(LogLevel.IMPORTANT, `[emit] name:${name} // DiscordMessage.type: ${dataLog.type} // DiscordMessage.msg: ${dataLog.message} // DiscordMessage.embeds: ${dataLog.embeds}`);
                }
                (data: ServerState) => {
                    const dataLog = data;
                    this?.log?.log(LogLevel.IMPORTANT, `[emit] name:${name} // ServerState: ${dataLog}`);
                }
                (data: ModUpdatedStatus) => {
                    const dataLog = data;
                    this?.log?.log(LogLevel.IMPORTANT, `[emit] name:${name} // ModUpdatedStatus.ID: ${dataLog.modIds} // ModUpdatedStatus.success: ${dataLog.success}`);
                }
                (data: GameUpdatedStatus) => {
                    const dataLog = data;
                    this?.log?.log(LogLevel.IMPORTANT, `[emit] name:${name} // GameUpdatedStatus.success: ${dataLog.success}`);
                }
            }
            this.EVENT_EMITTER.emit(name, data);
        }catch(e){
            console.log(`[event-bus] emit() ERORR: ${e}`);
        }
    }

    public on(name: InternalEventTypes.DISCORD_MESSAGE, listener: (message: DiscordMessage) => Promise<any>): Listener;
    public on(name: InternalEventTypes.MONITOR_STATE_CHANGE, listener: (newState: ServerState, previousState: ServerState) => Promise<any>): Listener;
    public on(name: InternalEventTypes.METRIC_ENTRY, listener: (metricEntryEvent: MetricEntryEvent) => Promise<any>): Listener;
    public on(name: InternalEventTypes.LOG_ENTRY, listener: (logEntryEvent: LogEntryEvent) => Promise<any>): Listener;
    public on(name: InternalEventTypes.MOD_UPDATED, listener: (status: ModUpdatedStatus) => Promise<any>): Listener;
    public on(name: InternalEventTypes.GAME_UPDATED, listener: (status: GameUpdatedStatus) => Promise<any>): Listener;
    public on(
        name: InternalEventTypes.GET_INTERNAL_MODS
        | InternalEventTypes.INTERNAL_MOD_INSTALL,
        listener: () => Promise<any>,
    ): Listener;
    public on(name: InternalEventTypes, listener: (...data: any[]) => Promise<any>): Listener {
        try{
            return this.EVENT_EMITTER.on(name, listener, { objectify: true }) as Listener;
        }catch(e){
            console.log(`[event-bus] on() ERORR: ${e}`);
        }
    }

    public clear(name: InternalEventTypes): void {
        try{
            this.EVENT_EMITTER.removeAllListeners(name);
        }catch(e){
            console.log(`[event-bus] clear() ERORR: ${e}`);
        }
    }

}
