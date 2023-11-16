import { GuildChannel, Message } from 'discord.js';
import { Manager } from '../control/manager';
import { LogLevel } from '../util/logger';
import { Request, RequestTemplate } from '../types/interface';
import { IService } from '../types/service';
import { LoggerFactory } from '../services/loggerfactory';
import { injectable, singleton } from 'tsyringe';
import { Interface } from './interface';

@singleton()
@injectable()
export class DiscordMessageHandler extends IService {

    public readonly PREFIX = '!';

    public constructor(
        loggerFactory: LoggerFactory,
        private manager: Manager,
        private eventInterface: Interface,
    ) {
        super(loggerFactory.createLogger('DiscordMsgHandler'));
    }

    private formatCommandUsage(command: string, template: RequestTemplate): string {
        return this.PREFIX + command
        + ' '
        + template.params
            .map((param) => param.optional ? `[${param.name}]` : `<${param.name}>`)
            .join(' ')
    }

    public async handleCommandMessage(message: Message): Promise<void> {
        if (!this.manager.initDone) {
            this.log.log(LogLevel.DEBUG, `Server chưa khởi tạo xong, không thể gọi lệnh!`);
            return;
        }

        const args = message.content.slice(this.PREFIX.length).trim().split(/ +/);
        const command = args.shift()?.toLowerCase();

        if (!command) {
            return;
        }

        const channelName = (message.channel as GuildChannel).name;
        const authorId = message.author.tag;

        if (!authorId?.includes('#')) {
            // safety
            this.log.log(LogLevel.DEBUG, `Bạn không có quyền gọi lệnh này!`);
            return;
        }

        const argsMessage = args?.join(' ');
        this.log.log(LogLevel.INFO, `Command "${command}" from "${authorId}" in "${channelName}" with args: "${argsMessage}"`);

        const configChannel = this.manager.config.discordChannels.find((x) => x.channel.toLowerCase() === channelName?.toLowerCase());
        if (command === 'help' && (configChannel.mode === 'admin')) {
            let response = 'List lệnh cho discord admin: \n\n';
            response += [...this.eventInterface.commandMap.entries()]
                .filter((x) => !x[1].disableDiscord)
                .map((x) => this.formatCommandUsage(...x))
                .join('\n');
            response += '\n\n (Trong <> là bắt buộc phải có, [] thì tuỳ.)';
            await message.reply(response);
            return;
        }

        const handler = this.eventInterface.commandMap?.get(command);
        if (!handler || handler.disableDiscord) {
            await message.reply('Sai cú pháp.');
            return;
        }

        if (configChannel?.mode !== 'admin' && !handler.discordPublic) {
            await message.reply('Dùng cho đúng chanel bạn êi!');
            return;
        }

        const req = new Request();
        req.accept = 'text/plain';
        req.resource = command;
        req.user = authorId;
        req.body = {};
        req.query = {};
        req.canStream = true;

        const templateParams = (handler.params || []);
        for (let i = 0; i < templateParams.length; i++) {
            if (i >= args.length) {
                await message.reply(`Sai cú pháp. Usage: ${this.formatCommandUsage(command, handler)}`);
                return;
            }
            try {
                const val = templateParams[i].parse ? templateParams[i].parse(args[i]) : args[i];
                req[templateParams[i].location || 'body'][templateParams[i].name] = val;
                if(command === 'global') {
                    req[templateParams[i].location || 'body'][templateParams[i].name] = argsMessage;
                }
            } catch {
                await message.reply(`Lỗi gọi cú pháp: '${templateParams[i].name}'. Usage: ${this.formatCommandUsage(command, handler)}`);
                return;
            }
        }

        const res = await this.eventInterface.execute(
            req,
            /* istanbul ignore next */ (part) => message.reply(`\n${part.body}`),
        );

        if (res.status >= 200 && res.status < 300) {
            // eslint-disable-next-line @typescript-eslint/no-base-to-string
            await message.reply(`\n${res.body}`);
        } else {
            // eslint-disable-next-line @typescript-eslint/no-base-to-string
            await message.reply(`\nError: ${res.body}`);
        }
    }

}
