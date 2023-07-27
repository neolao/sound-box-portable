import { argv } from 'node:process';
import { readFile } from 'node:fs/promises';
import sharpBmp from 'sharp-bmp';

const inputFilePath = argv[2];
const buffer = await readFile(inputFilePath);
const bitmap = sharpBmp.decode(buffer);
console.log(bitmap);