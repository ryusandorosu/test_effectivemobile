#include <iostream>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
  #include <windows.h>
#else
  #include "structs.h"
#endif

class BmpReader {
private:
  BITMAPFILEHEADER fileHeader;
  BITMAPINFOHEADER infoHeader;
  std::vector<std::vector<bool>> bitmapData;
  std::ifstream bmpFile;

public:
  void openBMP(const std::string& fileName) {
    bmpFile.open(fileName, std::ios::binary);
    if (!bmpFile) {
      throw std::runtime_error("Could not open BMP file.");
    }

    // чтение заголовка файла
    char headerBuffer[sizeof(BITMAPFILEHEADER)];
    bmpFile.read(headerBuffer, sizeof(headerBuffer));
    std::memcpy(&fileHeader, headerBuffer, sizeof(BITMAPFILEHEADER));

    if (fileHeader.bfType != 0x4D42) { // проверка на 'BM'
      throw std::runtime_error("Not BMP file.");
    }

    // чтение заголовка изображения
    char infoBuffer[sizeof(BITMAPINFOHEADER)];
    bmpFile.read(infoBuffer, sizeof(infoBuffer));
    std::memcpy(&infoHeader, infoBuffer, sizeof(BITMAPINFOHEADER));

    if (infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32) {
      throw std::runtime_error("Only 24 or 32 bit BMP files are supported.");
    }

    // чтение данных изображения
    int width = infoHeader.biWidth;
    int height = infoHeader.biHeight;
    bitmapData.resize(height, std::vector<bool>(width));

    // переход к пиксельным данным
    bmpFile.seekg(fileHeader.bfOffBits, std::ios::beg);

    for (int i = height - 1; i >= 0; --i) { // так как пиксели хранятся в обратном порядке
      for (int j = 0; j < width; ++j) {
        char rgb[3];
        bmpFile.read(rgb, 3); // чтение по 3 байта для RGB
        if (static_cast<unsigned char>(rgb[0]) == 0 && 
            static_cast<unsigned char>(rgb[1]) == 0 && 
            static_cast<unsigned char>(rgb[2]) == 0) {
          bitmapData[i][j] = false;  // черный цвет
        } else if (static_cast<unsigned char>(rgb[0]) == 255 && 
                  static_cast<unsigned char>(rgb[1]) == 255 && 
                  static_cast<unsigned char>(rgb[2]) == 255) {
          bitmapData[i][j] = true; // белый цвет
        } else {
          throw std::runtime_error("BMP file contains colors other than black or white.");
        }
      }
    }
  }

  void displayBMP() const {
    for (const auto& row : bitmapData) {
      for (bool pixel : row) {
        std::cout << (pixel ? "##" : "  "); // белый или черный
      }
      std::cout << std::endl;
    }
  }

  void closeBMP() {
    if (bmpFile.is_open()) {
      bmpFile.close();
    }
  }

  ~BmpReader() {
    closeBMP();
  }
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_file.bmp>" << std::endl;
    return 1;
  }

  try {
    BmpReader bmpReader;
    bmpReader.openBMP(argv[1]);
    bmpReader.displayBMP();
    bmpReader.closeBMP();
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
