import { opendir, writeFile } from 'node:fs/promises';
import { argv } from 'node:process';
import sharp from 'sharp';
import bmp from 'sharp-bmp';

const menuFilePath = argv[2];
const sourcePath = argv[3];
const destinationPath = argv[4];
try {
  const directory = await opendir(sourcePath);
  for await (const directoryEntry of directory) {
    if (directoryEntry.name.endsWith('.png')) {
      console.log(directoryEntry.name);
      const { data, info } = await sharp(`${sourcePath}/${directoryEntry.name}`)
        .composite([
          { input: menuFilePath }
        ])
        .flatten({ background: "#ffffff" })
        .ensureAlpha()
        .raw()
        .toBuffer({ resolveWithObject: true });
      const bmpData = {
          data,
          width: info.width,
          height: info.height
      };
      const rawData = bmp.encode(bmpData);
      await writeFile(`${destinationPath}/test.bmp`, rawData.data);
    }
  }
} catch (error) {
  console.error(error);
} 