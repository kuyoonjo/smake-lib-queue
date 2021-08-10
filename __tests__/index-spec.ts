import * as index from '../src/index';

test('Should have queue available', () => {
  expect(index.queue).toBeTruthy();
});
