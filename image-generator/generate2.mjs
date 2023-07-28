import { opendir, writeFile } from 'node:fs/promises';
import path from 'node:path'; 
import { argv } from 'node:process';
import sharp from 'sharp';
import bmpEncode from './bmpEncode.js';
import Jimp from 'jimp';

const backgroundFilePath = argv[2];
const menuFilePath = argv[3];
const sourcePath = argv[4];
const destinationPath = argv[5];

const font = await Jimp.loadFont(Jimp.FONT_SANS_16_BLACK);
try {
  const directoryName = path.basename(sourcePath);
  console.log(directoryName);
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
      const destinationFilePath = `${destinationPath}/${destinationFileName}`;
      await writeFile(destinationFilePath, rawData.data);

      // Add text
      const image = await Jimp.read(destinationFilePath);
      image.print(font, 170, 48, directoryName);
      //await image.writeAsync(destinationFilePath);
      const bufferWithText = await image.getBufferAsync(Jimp.MIME_BMP);
      const rawDataWithText = bmpEncode({ data: bufferWithText, width: info.width, height: info.height });
      await writeFile(destinationFilePath, rawDataWithText.data);

      console.log(`${sourcePath}/${directoryEntry.name}`, '=>', destinationFilePath);
    }
  }
} catch (error) {
  console.error(error);
} 