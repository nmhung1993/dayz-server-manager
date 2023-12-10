import {
    TextChannel,
    Client,
    Message,
    MessageEmbed,
    Intents   
} from 'discord.js';
import { DiscordMessageHandler } from '../interface/discord-message-handler';
import { IStatefulService } from '../types/service';
import { LogLevel } from '../util/logger';
import { Manager } from '../control/manager';
import { injectable, singleton } from 'tsyringe';
import { LoggerFactory } from './loggerfactory';
import { EventBus } from '../control/event-bus';
import { InternalEventTypes } from '../types/events';
import { DiscordMessage } from '../types/discord';

@singleton()
@injectable()
export class DiscordBot extends IStatefulService {

    public client: Client | undefined;
    private ready = false;

    private msgQueue: DiscordMessage[] = [];

    public debug: boolean = true;

    public constructor(
        loggerFactory: LoggerFactory,
        private manager: Manager,
        private messageHandler: DiscordMessageHandler,
        private eventBus: EventBus,
    ) {
        super(loggerFactory.createLogger('Discord'));

        this.eventBus.on(
            InternalEventTypes.DISCORD_MESSAGE,
            /* istanbul ignore next */ (message: DiscordMessage) => this.sendMessage(message),
        );
    }

    public async start(): Promise<void> {

        if (!this.manager.config.discordBotToken) {
            this.log.log(LogLevel.WARN, 'Not starting discord bot, because no bot token was provided');
            return;
        }

        const client = new Client({
            intents: [
                Intents.FLAGS.GUILDS, 
                Intents.FLAGS.GUILD_MESSAGES 
            ]
        });
        client.on('ready', () => this.onReady());
        if (this.debug) {
            client.on('invalidated', () => this.log.log(LogLevel.ERROR, 'invalidated'));
            client.on('debug', (m) => this.log.log(LogLevel.DEBUG, m));
            client.on('warn', (m) => this.log.log(LogLevel.WARN, m));
        }
        client.on('messageCreate', (m) => {
            if (m.content.toLowerCase() == 'ping') {
                m.reply('pong');
            } else
                this.onMessage(m);
        });
        client.on('disconnect', (d) => {
            if (d?.wasClean) {
                this.log.log(LogLevel.INFO, 'disconnect');
            } else {
                this.log.log(LogLevel.ERROR, 'disconnect', d);
            }
        });
        client.on('error', (e) => this.log.log(LogLevel.ERROR, 'error', e));

        try {
            this.log.log(LogLevel.IMPORTANT, `Starting login discord bot with token: ${this.manager.config.discordBotToken}`);
            await client.login(this.manager.config.discordBotToken);
            this.client = client;
            this.sendQueuedMessage();
        } catch (e) {
            this.log.log(LogLevel.WARN, 'Not starting discord bot, login failed', e);
        }
    }

    private onReady(): void {
        this.log.log(LogLevel.IMPORTANT, 'Discord Ready!');
        this.ready = true;
        this.sendQueuedMessage();
    }

    private sendQueuedMessage(): void {
        setTimeout(() => {
            const msgQueue = this.msgQueue;
            this.msgQueue = [];
            for (const msg of msgQueue) {
                this.sendMessage(msg);
            }
        }, 1000);
    }

    private onMessage(message: Message) : void {
        if (message.author.bot) {
            return;
        }

        if (this.debug) {
            this.log.log(LogLevel.DEBUG, `Detected message: ${message.content}`);
        }

        if (message.content?.startsWith(this.messageHandler.PREFIX)
            && message.channelId === this.manager.config.channelAdmin) {
            void this.messageHandler.handleCommandMessage(message);
        }
    }

    public async stop(): Promise<void> {
        this.ready = false;
        if (this.client) {
            await this.client.destroy();
            this.client = undefined;
        }
    }

    public async sendMessage(message: DiscordMessage): Promise<void> {

        if (!this.client || !this.ready) {
            this.log.log(LogLevel.WARN, `Queueing message because client did not start or is not yet ready`, this.ready);
            this.msgQueue.push(message);
            return;
        }

        let TargetChannel;

        if(message.type.toString() === 'notification'){
            TargetChannel = this.client.channels.cache.get(`${this.manager.config.channelNoti}`) as TextChannel;
        }else if(message.type.toString() === 'admin'){
            TargetChannel = this.client.channels.cache.get(`${this.manager.config.channelAdmin}`) as TextChannel;
        }else if(message.type.toString() === 'rcon'){
            TargetChannel = this.client.channels.cache.get(`${this.manager.config.channelRCON}`) as TextChannel;
        }


        if(message.embed != undefined) {
            this.log.log(LogLevel.IMPORTANT, `embed : ${JSON.stringify(message.embed)}`);
            TargetChannel?.send({embed:message.embed})
        } else if(message.embeds != undefined) {
            this.log.log(LogLevel.IMPORTANT, `embeds : ${JSON.stringify(message.embeds)}`);
            TargetChannel?.send({embeds:message.embeds})
        } else if(message.message != '' && message.message != undefined){
            this.log.log(LogLevel.IMPORTANT, `message : ${message.message}`);
            TargetChannel?.send(`${message.message}`);
            //TargetChannel?.send({content: message.message});
        }
    }

}
