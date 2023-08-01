import { opendir, writeFile } from 'node:fs/promises';
import path from 'node:path'; 
import { argv } from 'node:process';
import sharp from 'sharp';
import bmpEncode from './bmpEncode.js';

const backgroundFilePath = argv[2];
const menuFilePath = argv[3];
const sourcePath = argv[4];
const destinationPath = argv[5];

try {
  const directoryName = path.basename(sourcePath);
  console.log(directoryName);
  const directory = await opendir(sourcePath);
  for await (const directoryEntry of directory) {
    const fileName = directoryEntry.name.substring(0, directoryEntry.name.length - 4);
    if (directoryEntry.name.endsWith('.png')) {
      const textOptions = {
        fill: '#000000',
        font: 'Arial', 
        size: 20,
      };
      const buffer = await sharp(backgroundFilePath)
        .composite([
          { input: `${sourcePath}/${directoryEntry.name}` },
          { input: menuFilePath },
          // Folder name
          {
            input: Buffer.from(`<svg height="200" width="200">
                <text x="165" y="62" fill="${textOptions.fill}" font-family="${textOptions.font}" font-size="${textOptions.size}">${directoryName}</text>
                <text x="160" y="162" fill="${textOptions.fill}" font-family="${textOptions.font}" font-size="${textOptions.size}">${fileName}</text>
              </svg>`)
          }
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

      const destinationFileName = `${fileName}.bmp`;
      const destinationFilePath = `${destinationPath}/${destinationFileName}`;
      await writeFile(destinationFilePath, rawData.data);

      console.log(`${sourcePath}/${directoryEntry.name}`, '=>', destinationFilePath);
    }
  }
} catch (error) {
  console.error(error);
}