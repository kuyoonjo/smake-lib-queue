import { resolve } from 'path';
import { LLVM } from '@smake/llvm';

export abstract class LibQueue {
  static config(llvm: LLVM) {
    llvm.includedirs = [
      ...llvm.includedirs,
      resolve(__dirname, '..', 'include').replace(/\\/g, '/'),
    ];
  }
}
