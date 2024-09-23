#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <stdexcept>

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
        bmpFile.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
        if (fileHeader.bfType != 0x4D42) { // проверка на 'BM'
            throw std::runtime_error("Not BMP file.");
        }

        // чтение заголовка изображения
        bmpFile.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

        if (infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32) {
            throw std::runtime_error("Only 24 or 32 bit BMP files are supported.");
        }

        // чтение данных изображения
        int width = infoHeader.biWidth;
        int height = infoHeader.biHeight;
        // int rowSize = (width * (infoHeader.biBitCount / 8) + 3) & (~3); // выровненный размер строки

        bitmapData.resize(height, std::vector<bool>(width));

        // переход к пиксельным данным
        bmpFile.seekg(fileHeader.bfOffBits, std::ios::beg);

        for (int i = height - 1; i >= 0; --i) { // так как пиксели хранятся в обратном порядке
            for (int j = 0; j < width; ++j) {
                unsigned char rgb[3];
                bmpFile.read(reinterpret_cast<char*>(&rgb), 3);
                if (rgb[0] == 0 && rgb[1] == 0 && rgb[2] == 0) {
                    bitmapData[i][j] = false;  // черный цвет
                } else if (rgb[0] == 255 && rgb[1] == 255 && rgb[2] == 255) {
                    bitmapData[i][j] = true; // белый цвет
                } else {
                    throw std::runtime_error("BMP file contains colors other than black or white.");
                }
            }
            // bmpFile.ignore(rowSize - width * 3); // пропуск паддинга
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
        std::cerr << "Usage: drawBmp.exe <path_to_file.bmp>\n";
        return 1;
    }

    try {
        BmpReader bmpReader;
        bmpReader.openBMP(argv[1]);
        bmpReader.displayBMP();
        bmpReader.closeBMP();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
