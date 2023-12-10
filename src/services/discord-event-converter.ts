/* istanbul ignore file */
/* Not unit testing this class because it is purely integration */

import { injectable, singleton } from "tsyringe";
import { IService } from "../types/service";
import { LoggerFactory } from "./loggerfactory";
import { Manager } from "../control/manager";
import { EventBus } from "../control/event-bus";
import { InternalEventTypes } from "../types/events";
import { SteamMetaData } from "./steamcmd";
import { MessageEmbed } from "discord.js";
import { GameUpdatedStatus, ModUpdatedStatus } from "../types/steamcmd";
import { ServerState } from "../types/monitor";
import { LogLevel } from "../util/logger";
import { DiscordBot } from "./discord";

@singleton()
@injectable()
export class DiscordEventConverter extends IService {

    public constructor(
        loggerFactory: LoggerFactory,
        //log: IService,
        private manager: Manager,
        private steamMetaData: SteamMetaData,
        private eventBus: EventBus,
    ) {
        super(loggerFactory.createLogger('DiscordHandler'));

        this.eventBus.on(
            InternalEventTypes.MOD_UPDATED,
            this.handleModUpdated,
        );

        this.eventBus.on(
            InternalEventTypes.GAME_UPDATED,
            this.handleGameUpdated,
        );

        this.eventBus.on(
            InternalEventTypes.MONITOR_STATE_CHANGE,
            this.handleServerState,
        )
    }

    private async handleGameUpdated(status: GameUpdatedStatus): Promise<void> {
        try{
            this?.log?.log(LogLevel.INFO, `handleGameUpdated() - status: ${status}`);
            if (!status.success) {
                this.eventBus.emit(
                    InternalEventTypes.DISCORD_MESSAGE,
                    {
                        type: 'admin',
                        message: 'Update server thất bại!!!',
                    },
                );
            } else {
                this.eventBus.emit(
                    InternalEventTypes.DISCORD_MESSAGE,
                    {
                        type: 'notification',
                        message: 'Server đã Update thành công!',
                    },
                );
            }
        }catch(e){
            this?.log?.log(LogLevel.ERROR, `handleGameUpdated() - Something wrong ${e}`);
        }
    }

    public async handleModUpdated(status: ModUpdatedStatus): Promise<void> {
        try{
            this?.log?.log(LogLevel.INFO, `[handleModUpdated] - status: ${JSON.stringify(status)}`);
            if (!status.success) {
                this.eventBus.emit(
                    InternalEventTypes.DISCORD_MESSAGE,
                    {
                        type: 'admin',
                        message: `Có lỗi khi update mods: ${status.modIds.join('\n')}`,
                    },
                );
                return;
            }

            const modInfos = await this.steamMetaData.getModsMetaData(status.modIds);

            // modInfos.forEach(function (modDetail) {
            //     console.log(`[handleModUpdated] - modInfo: ${modDetail !== undefined ? JSON.stringify(modDetail) : "undefined"}`);
            //     //this.log.log(LogLevel.INFO, `[handleModUpdated] - modInfo: ${modDetail == undefined ? JSON.stringify(modDetail) : "undefined"}`);
            // });
            new Date().toLocaleString('VN', { timeZone: 'Asia/ho_chi_minh' });
            const embeds = status.modIds
                .map((modId) => {
                    return modInfos.find((modInfo) => modInfo?.publishedfileid === modId) || modId;
                })
                .map((modInfo) => {
                    const fields = [];
                    if (typeof modInfo !== 'string' && modInfo?.title) {
                        if (modInfo.time_updated || modInfo.time_created) {
                            fields.push({
                                name: 'Uploaded at',
                                value: (new Date((modInfo.time_updated || modInfo.time_created) * 1000))
                                    .toLocaleString('VN', { timeZone: 'Asia/ho_chi_minh' })
                                    .split(/[T\.]/)
                                    .slice(0, 2)
                                    .join(' ')
                                    + ' GMT+7',
                                inline: true,
                            });
                        }
                        const embed = new MessageEmbed({
                            color: 0x0099FF,
                            title: `Mod updated: ${modInfo.title}`,
                            url: `https://steamcommunity.com/sharedfiles/filedetails/?id=${modInfo.publishedfileid}`,
                            fields,
                            thumbnail: { url: modInfo.preview_url || undefined },
                            image: { url: modInfo.preview_url || undefined },
                            footer: {
                                text: '',
                            },
                        });
                        return embed;
                    } else if (typeof modInfo === 'string') {
                        return new MessageEmbed({
                            color: 0x0099FF,
                            title: `Mod updated: ${modInfo}`,
                            url: `https://steamcommunity.com/sharedfiles/filedetails/?id=${modInfo}`,
                            footer: {
                                text: '',
                            },
                        });
                    }
                    return null;
                })
            .filter((x) => !!x);

            for(let embed of  embeds){
                this?.log?.log(LogLevel.INFO,`[handleModUpdated] - sending discord embed: ${embed !== undefined ? JSON.stringify(embed) : `undefined = ${embed == undefined}`}`);
                this.eventBus.emit(
                    InternalEventTypes.DISCORD_MESSAGE,
                    {
                        type: 'notification',
                        embeds: [embed],
                        //message: `${embed.title} \n ${embed.url}` //JSON.stringify(embeds),
                    }
                );
            }

        }catch(e){
            this?.log?.log(LogLevel.ERROR, `[handleModUpdated] - Something wrong ${e}`);
        }
    }


    public async handleServerState(newState: ServerState, previousState: ServerState): Promise<void> {
        try{
            this?.log?.log(LogLevel.INFO, `[handleServerState] - newState: ${newState} // previousState: ${previousState}`);
            // msg about server startup
            if ((newState === ServerState.STARTED)&&(previousState === ServerState.STARTING)) {
                const message = `Server đã khởi động! <@&${this.manager.config.discordRoleID}>`;
                this?.log?.log(LogLevel.IMPORTANT, message);
                this.eventBus.emit(
                    InternalEventTypes.DISCORD_MESSAGE,
                    {
                        type: 'notification',
                        message,
                    },
                );
            }

            // msg about server stop
            if ((newState === ServerState.STOPPED)&&(previousState === ServerState.STOPPING)) {
                const message = 'Server đã dừng!';
                this?.log?.log(LogLevel.IMPORTANT, message);
                this.eventBus.emit(
                    InternalEventTypes.DISCORD_MESSAGE,
                    {
                        type: 'notification',
                        message,
                    },
                );
            }

            // handle stop after running
            if (
                newState === ServerState.STOPPED
                && (
                    previousState === ServerState.STARTING
                    || previousState === ServerState.STARTED
                )
            ) {
                const message = `Có gì đó không ổn rồi man <@${this.manager.config.discordAdminID}>. Đang thử khởi động lại server...`;
                this?.log?.log(LogLevel.WARN, message);
                this.eventBus.emit(
                    InternalEventTypes.DISCORD_MESSAGE,
                    {
                        type: 'admin',
                        message,
                    },
                );
            }
        }catch(e){
            this?.log?.log(LogLevel.ERROR, `[handleServerState] - Something wrong ${e}`);
        }
    }
}