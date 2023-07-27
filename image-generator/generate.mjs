import { opendir, writeFile } from 'node:fs/promises';
import { argv } from 'node:process';
import sharp from 'sharp';
import bmpEncode from './bmpEncode.js';

const backgroundFilePath = argv[2];
const menuFilePath = argv[3];
const sourcePath = argv[4];
const destinationPath = argv[5];
try {
  const directory = await opendir(sourcePath);
  for await (const directoryEntry of directory) {
    if (directoryEntry.name.endsWith('.png')) {
      const buffer = await sharp(backgroundFilePath)
        .composite([
          { input: `${sourcePath}/${directoryEntry.name}` },
          { input: menuFilePath }
        ])
        .flatten({ background: "#ffffff" })
        .ensureAlpha()
        .toBuffer();
      const { data, info } = await sharp(buffer).flip().raw().toBuffer({ resolveWithObject: true });
      const bmpData = {
          data,
          width: info.width,
          height: info.height
        };
      const rawData = bmpEncode(bmpData);

      const destinationFileName = `${directoryEntry.name.substring(0, directoryEntry.name.length - 4)}.bmp`;
      await writeFile(`${destinationPath}/${destinationFileName}`, rawData.data);

      console.log(`${sourcePath}/${directoryEntry.name}`, '=>', `${destinationPath}/${destinationFileName}`);
    }
  }
} catch (error) {
  console.error(error);
} 