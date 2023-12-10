import * as fs from 'fs';
import * as path from 'path';
import { Monitor } from '../services/monitor';
import { Manager } from '../control/manager';

export const test = undefined;

export async function cleanLogFiles () {
    const targetFolder = './DayZServer/profiles';
    const files = fs.readdirSync(targetFolder);

    const logFiles = files.filter(file => { 
        // console.log('basename = '+path.basename(file).toLowerCase());
        if (path.basename(file).toLowerCase().match(/^(scriptext.log|dayzserver_x64.adm)$/)) {
            return false;
        }
        if (path.extname(file).toLowerCase().match(/^(.log|.adm|.rpt|.mdmp)$/)) {
            return true;
        }
    });

    // console.log('logFiles = '+logFiles.length);

    if (!fs.existsSync('DayZServer/profiles/logs')) {
        fs.mkdirSync('DayZServer/profiles/logs');
    }

    logFiles.forEach(
        (param) => {
            // console.log('Moving logFile: '+param);
            fs.renameSync('DayZServer/profiles/'+param,'DayZServer/profiles/logs/'+param);
        },
    );
}