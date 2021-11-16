const { LLVM } = require('smake');
const { LibQueue } = require('./lib');

const test = new LLVM('test', 'arm64-apple-darwin');
test.files = ['src/test.cc'];
LibQueue.config(test);

module.exports = [test];
