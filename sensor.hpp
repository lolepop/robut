#define BUFFER_SIZE 10
int buffer[BUFFER_SIZE];
int bufLen, rBuf;

int medianFilter(int* buffer, int bufferLen) {
  int tmpBuf[BUFFER_SIZE];
  for (int i = 0; i < bufLen; i++)
    tmpBuf[i] = buffer[i];
  for (int i = 0; i < bufLen; i++) {
    for (int j = 0; j < bufLen-i; j++) {
      if (tmpBuf[j-1] > tmpBuf[j]) {
        int t = tmpBuf[j-1];
        tmpBuf[j-1] = tmpBuf[j];
        tmpBuf[j] = t;
      }
    }
  }
  return bufLen == 1 ?
    tmpBuf[bufLen / 2] :
    ((tmpBuf[bufLen / 2] + tmpBuf[bufLen / 2 - 1]) / 2);
}

int appendSensorData(int micros) {
  if(micros <= 0) return;

  buffer[rBuf] = micros;
  rBuf = (rBuf + 1) % BUFFER_SIZE;
  if (bufLen < BUFFER_SIZE)
    bufLen++;
  return medianFilter(buffer, bufLen);
}