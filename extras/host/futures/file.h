#ifndef MCUCORE_EXTRAS_HOST_FUTURES_FILE_H_
#define MCUCORE_EXTRAS_HOST_FUTURES_FILE_H_

// The File interface exposed by the Arduino SD Card Library.

namespace SDLib {

class File : public Stream {
 private:
  char _name[13];  // our name
  SdFile *_file;   // underlying file pointer

 public:
  File(SdFile f, const char *name);  // wraps an underlying SdFile
  File(void);                        // 'empty' constructor
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int read();
  virtual int peek();
  virtual int available();
  virtual void flush();
  int read(void *buf, uint16_t nbyte);
  boolean seek(uint32_t pos);
  uint32_t position();
  uint32_t size();
  void close();
  operator bool();
  char *name();

  boolean isDirectory(void);
  File openNextFile(uint8_t mode = O_RDONLY);
  void rewindDirectory(void);

  using Print::write;
};

}  // namespace SDLib

#endif  // MCUCORE_EXTRAS_HOST_FUTURES_FILE_H_
