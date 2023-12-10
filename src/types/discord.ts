import { MessageEmbed } from "discord.js";
import { DiscordChannelType } from "../config/config";

export interface DiscordMessage {
    type?: DiscordChannelType,
    message?: string,
    channelID?: string,
    embeds?: MessageEmbed[],
    embed?: MessageEmbed,
}

export const isDiscordChannelType = (test: DiscordChannelType | DiscordChannelType[], wanted: DiscordChannelType): boolean => {
    return Array.isArray(test) ? test.includes(wanted) : test === wanted;
}