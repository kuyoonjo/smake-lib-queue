const { LLVM } = require('@smake/llvm');
const { vscode } = require('@smake/llvm-vscode');
const { LibQueue } = require('./lib');

const test = new LLVM('test', 'arm64-apple-darwin');
test.files = ['src/test.cc'];
LibQueue.config(test);
vscode(test);

module.exports = [test];
