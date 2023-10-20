// bmp_editor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;


#pragma pack(2)

//Заголовок файла BMP 
typedef struct tBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
}sFileHead;

//Заголовок BitMap's
typedef struct tBITMAPINFOHEADER
{
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
}sInfoHead;

sFileHead FileHead;
sInfoHead InfoHead;

typedef struct tMyBITMAPFILEHEADER
{
	WORD bfType; //Идентификатор типа файла (2 байта)
	CHAR comprationType; // Тип сжатия (1 байт)
	CHAR nameOfProgram[8]; // Название программы, создающей файлы данного формата (8 байт)
	DWORD sizeImage; // размер изображения в пикселях (4 байта)
	DWORD height; // Высота изображения в пикселях (4 байта)
	BYTE depth; // Глубина цвета (количество бит на 1 пиксель) (1 байт)
	WORD size; // Размер заголовка в байтах (2 байта)
}sMyFileHead;

sMyFileHead MyFileHead;
//Пиксель
struct Color
{
	BYTE blue;
	BYTE green;
	BYTE red;
};

//Размер 1-го пикселя
int pixel_size = sizeof(Color);


//1 - BMP, 2 - CMP
int img_type = 0;

//Исходное изображение
Color *src_image = 0;
//Результативное изображение
Color *dst_image = 0;

//Размер изображения
int width = 0;
int height = 0;

//Вывести заголовок BMP файла
void ShowBMPHeaders(tBITMAPFILEHEADER fh, tBITMAPINFOHEADER ih)
{
	cout<<"Type: "<<(CHAR)fh.bfType<<endl;
	cout<<"Size: "<<fh.bfSize<<endl;
	cout<<"Shift of bits: "<<fh.bfOffBits<<endl;
	cout<<"Width: "<<ih.biWidth<<endl;
	cout<<"Height: "<<ih.biHeight<<endl;
	cout<<"Planes: "<<ih.biPlanes<<endl;
	cout<<"BitCount: "<<ih.biBitCount<<endl;
	cout<<"Compression: "<<ih.biCompression<<endl;
}

void ShowMyHeaders(tMyBITMAPFILEHEADER mh)
{
	cout << "Type: " << (CHAR)mh.bfType << endl;
	cout << "Compration: " << (int)mh.comprationType << endl;
	cout << "NameOfProgram: " << mh.nameOfProgram << endl;
	cout << "SizeOfImage: " << mh.sizeImage << endl;
	cout << "Height: " << mh.height << endl;
	cout << "Depth: " << (int)mh.depth << endl;
	cout << "Size: " << mh.size << endl;
}

//Функция для загрузки изображения
bool OpenImage(string path)
{
	ifstream img_file;
	Color temp;
	char buf[3];

	//Открыть файл на чтение
	img_file.open(path.c_str(), ios::in | ios::binary);
	if (!img_file)
	{
		cout<<"File isn`t open!"<<endl;
		return false;
	}

		//Считать заголовки BMP
		img_file.read((char*)&FileHead, sizeof(FileHead));
		img_file.read((char*)&InfoHead, sizeof(InfoHead));
		
		img_type = 1;
		ShowBMPHeaders(FileHead, InfoHead);
		//Присвоить длину и ширину изображения
		width = InfoHead.biWidth;
		height = InfoHead.biHeight;


	//Выделить место под изображение
	src_image = new Color[width*height];

	int i,j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.read((char*)&temp, pixel_size);
			src_image[i*width + j] = temp;
		}
		//Дочитать биты используемые для выравнивания до двойного слова
		img_file.read((char*)buf, j%4);
	}
	img_file.close();

	return true;
}

bool OpenMyImage(string path)
{
	ifstream img_file;
	Color temp;
	char buf[3];

	//Открыть файл на чтение
	img_file.open(path.c_str(), ios::in | ios::binary);
	if (!img_file)
	{
		cout << "File isn`t open!" << endl;
		return false;
	}

	//Считать заголовки SDA
	img_file.read((char*)&MyFileHead, sizeof(MyFileHead));

	ShowMyHeaders(MyFileHead);
	//Присвоить длину и ширину изображения
	width = MyFileHead.sizeImage / MyFileHead.height;
	height = MyFileHead.height;


	//Выделить место под изображение
	src_image = new Color[width * height];

	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.read((char*)&temp, pixel_size);
			src_image[i * width + j] = temp;
		}
		//Дочитать биты используемые для выравнивания до двойного слова
		img_file.read((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}


//Функция для сохранения изображение
bool SaveImage(string path)
{
	ofstream img_file;
	char buf[3];

	//Открыть файл на запись
	img_file.open(path.c_str(), ios::out | ios::binary);
	if (!img_file)
	{
		return false;
	}

	img_file.write((char*)&FileHead, sizeof(FileHead));
	img_file.write((char*)&InfoHead, sizeof(InfoHead));
		
	//Скопировать из исходного в результирующее изображение
	dst_image = new Color[width*height];
	memcpy(dst_image, src_image, width*height*sizeof(Color));


	//Записать файл
	int i,j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.write((char*)&dst_image[i*width + j], pixel_size);
		}
		img_file.write((char*)buf, j%4);
	}
	img_file.close();
	
	return true;
}

//Скопировать содержимое результируещего изображения в начальное
void CopyDstToSrc()
{
	if (dst_image != 0)
	{
		memcpy(src_image, dst_image, width*height*sizeof(Color));
	}
}

// Функция для применения порогового фильтра с окном 3x1 к изображению
void Filter() {
	int threshold;
	cout << "input threshold: ";
	cin >> threshold;
	Color* filteredImage = new Color[width * height];
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int sumBlue = 0;
			int sumGreen = 0;
			int sumRed = 0;

			int averageBlue = 0;
			int averageGreen = 0;
			int averageRed = 0;

			if (j == 0) 
			{
				sumBlue += src_image[i * width + j + 1].blue;
				sumGreen += src_image[i * width + j + 1].green;
				sumRed += src_image[i * width + j + 1].red; 

				averageBlue = sumBlue;
				averageGreen = sumGreen;
				averageRed = sumRed;
			}
			else if (j == width)
			{
				sumBlue += src_image[i * width + j - 1].blue;
				sumGreen += src_image[i * width + j - 1].green;
				sumRed += src_image[i * width + j - 1].red;

				averageBlue = sumBlue;
				averageGreen = sumGreen;
				averageRed = sumRed;
			}
			else {
				sumBlue += src_image[i * width + j - 1].blue;
				sumGreen += src_image[i * width + j - 1].green;
				sumRed += src_image[i * width + j - 1].red;

				sumBlue += src_image[i * width + j + 1].blue;
				sumGreen += src_image[i * width + j + 1].green;
				sumRed += src_image[i * width + j + 1].red;

				averageBlue = sumBlue / 2;
				averageGreen = sumGreen / 2;
				averageRed = sumRed / 2;
			}

			if (abs(averageBlue - filteredImage[i * width + j].blue) > threshold || abs(averageGreen - filteredImage[i * width + j].green) > threshold || abs(averageRed - filteredImage[i * width + j].red) > threshold) {
				filteredImage[i * width + j].blue = averageBlue;
				filteredImage[i * width + j].green = averageGreen;
				filteredImage[i * width + j].red = averageRed;
			}
			else
			{
				filteredImage[i * width + j] = src_image[i * width + j];
			}
		}
	}

	memcpy(src_image, filteredImage, width * height * sizeof(Color));
	delete[] filteredImage;
}


//Зашумление изображения с заданной долей вероятности
void AddNoise(double probability)
{
	int size = width*height;
	int count = (int)(size*probability)/100;
	int x,y;
	long pos;
	for (int i = 0; i < count; i++)
	{ 
		x = rand()%width;
		y = rand()%height;
		pos = y*width+x; 
		src_image[pos].blue = rand() % 256;
		src_image[pos].green = rand() % 256;
		src_image[pos].red = rand() % 256;
	}
	cout<<"Point was added: "<<count<<endl;
}

//Отобразить текущее изображение с помощью вызова стандартного просмотрщика
void ShowImage(string path)
{
	ShowBMPHeaders(FileHead, InfoHead);
	system(path.c_str());
}

void ShowMyImage(string path)
{
	ShowMyHeaders(MyFileHead);
	system(path.c_str());
}

//Считать путь к изображению
void ReadPath(string &str)
{
	str.clear();
	cout<<"Enter path to image"<<endl;
	cin>>str;
}

bool BmpToMy(string path)
{
	ifstream img_file_in;
	Color temp;
	char buf[3];

	//Открыть файл на чтение
	img_file_in.open(path.c_str(), ios::in | ios::binary);
	if (!img_file_in)
	{
		cout << "File isn`t open!" << endl;
		return false;
	}

	//Считать заголовки BMP
	img_file_in.read((char*)&FileHead, sizeof(FileHead));
	img_file_in.read((char*)&InfoHead, sizeof(InfoHead));

	img_type = 1;
	ShowBMPHeaders(FileHead, InfoHead);
	//Присвоить длину и ширину изображения
	width = InfoHead.biWidth;
	height = InfoHead.biHeight;


	//Выделить место под изображение
	src_image = new Color[width * height];

	int j, i;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file_in.read((char*)&temp, pixel_size);
			src_image[i * width + j] = temp;
		}
		//Дочитать биты используемые для выравнивания до двойного слова
		img_file_in.read((char*)buf, j % 4);
	}
	img_file_in.close();

	MyFileHead.bfType = 'S';
	MyFileHead.comprationType = InfoHead.biCompression;
	strcpy(MyFileHead.nameOfProgram, "VisualS\0");
	MyFileHead.sizeImage = width * height;
	MyFileHead.height = InfoHead.biHeight;
	MyFileHead.depth = InfoHead.biBitCount;
	MyFileHead.size = sizeof(MyFileHead);

	ofstream img_file_out;

	//Открыть файл на запись
	ReadPath(path);
	img_file_out.open(path.c_str(), ios::out | ios::binary);
	if (!img_file_out)
	{
		return false;
	}

	img_file_out.write((char*)&MyFileHead, sizeof(MyFileHead));

	//Скопировать из исходного в результирующее изображение
	dst_image = new Color[width * height];
	memcpy(dst_image, src_image, width * height * sizeof(Color));


	//Записать файл
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file_out.write((char*)&dst_image[i * width + j], pixel_size);
		}
		img_file_out.write((char*)buf, j % 4);
	}
	img_file_out.close();

	return true;
}


void ClearMemory(void){
    //Освободить память исходного изображения
    if (src_image != 0)
    {
        delete [] src_image;
    }
    //Освободить память результрующего изображения
    if (dst_image != 0)
    {
        delete [] dst_image;
    }
}

BYTE brightness(Color color)
{
	return (BYTE)0.3 * color.red + 0.6 * color.green + 0.1 * color.blue;
}

void Wallace()
{
	double F0, F1, F2, F3, F4;
	double F;
	double p;

	cout << "Введите значение порога p ";
	cin >> p;

	Color* TempImage = new Color[width * height];
 
	for (int i = 1; i < height - 1; i++) 
	{
		for (int j = 1; j < width - 1; j++)
		{
			F0 = brightness(src_image[i * width + j]);
			F1 = brightness(src_image[(i - 1) * width + j]);
			F2 = brightness(src_image[(i + 1) * width + j]);
			F3 = brightness(src_image[i * width + j - 1]);
			F4 = brightness(src_image[i * width + j + 1]);

			F = log(pow(F0, 4) / (F1 * F2 * F3 * F4));


			if (F > p)
			{
				TempImage[i * width + j].red = 0;
				TempImage[i * width + j].green = 0;
				TempImage[i * width + j].blue = 0;
			}
			else
			{
				TempImage[i * width + j].red = 255;
				TempImage[i * width + j].green = 255;
				TempImage[i * width + j].blue = 255;
			}
		}
	}

	memcpy(src_image, TempImage, width * height * sizeof(Color));
	delete[] TempImage;
}

// Функция для создания прямоугольной рамки с градиентной заливкой
void AddGradientBorder(int Border_width, Color startColor, Color endColor) {

	Color* TempImagesrc = new Color[width * height];
	memcpy(TempImagesrc, src_image, width * height * sizeof(Color));

	src_image = new Color[(width + Border_width) * (height + Border_width)];
	Color* TempImage = new Color[(width + Border_width) * (height + Border_width)];

	for (int i = 0; i < width + Border_width; i++) {
		for (int j = 0; j < height + Border_width; j++) {
			double t = (double)(i) / height; 
			Color NewColor;
			NewColor.red = startColor.red + t * (endColor.red - startColor.red);
			NewColor.green = startColor.green + t * (endColor.green - startColor.green);
			NewColor.blue = startColor.blue + t * (endColor.blue - startColor.blue);

			TempImage[i * width + j] = NewColor;
		}
	}

	for (int i = Border_width; i < height - Border_width; i++) {
		for (int j = Border_width; j < width - Border_width; j++) {
			TempImage[i * width + j] = TempImagesrc[(i - Border_width) * width + j - Border_width];
		}
	}

	memcpy(src_image, TempImage, (width + Border_width) * (height + Border_width) * sizeof(Color));
	delete[] TempImage;
}


Color inputColor()
{
	int red, green, blue;
	Color TempColor;
	cout << "ColorRed" << endl;
	cin >> red;
	cout << "ColorGreen" << endl;
	cin >> green;
	cout << "ColorBlue" << endl;
	cin >> blue;
	TempColor.red = (byte)red;
	TempColor.green = (byte)green;
	TempColor.blue = (byte)blue;
	return TempColor;
}


int main(int argc, char* argv[])
{
    srand((unsigned)time( NULL ));

	setlocale(LC_ALL, "ru");

	int borderWidth; 
	Color startColor, endColor;

	byte red, green, blue;

	string path_to_image, temp, path_to_filtr_image;
	int percent;

	cout << "Выберите, что хотите сделать:\n1.открыть файл bmp и отредактировать его\n2.Перевести файл формата bmp в файл формата SDA\n3.Открыть SDA файл\n4.Границы у исходного изображения\n5.Границы у отфильтрованного\n6.Границы у зашумленного\n7.Границы у зашумленного отфильтрованного\nВаш выбор: \n";
	int mode;
	cin >> mode;
	switch (mode) {
	case 1:
		ReadPath(path_to_image);
		OpenImage(path_to_image);
		ShowImage(path_to_image);
		cout << "Введите проценты шума\n";
		cin >> percent;
		AddNoise(percent);
		cout << "Сохраняем добавленный шум в файл" << endl;
		ReadPath(temp);
		SaveImage(temp);
		ShowImage(temp);
		Filter();
		cout << "Сохраняем отфильтрованное изображение в файл" << endl;
		ReadPath(path_to_filtr_image);
		SaveImage(path_to_filtr_image);
		ShowImage(path_to_filtr_image);
		ClearMemory();
		break;
	case 2:
		ReadPath(path_to_image);
		BmpToMy(path_to_image);
		ClearMemory();
		break;
	case 3:
		ReadPath(path_to_image);
		OpenMyImage(path_to_image);
		ClearMemory();
		break;
	case 4:
		cout << "Исходное изображение." << endl;
		ReadPath(path_to_image);
		OpenImage(path_to_image);
		Wallace();
		cout << "Сохранение изображения" << endl;
		ReadPath(temp);
		SaveImage(temp);
		ShowImage(temp);
		break;
	case 5:
		cout << "Исходное отфильтрованное изображение." << endl;
		ReadPath(path_to_image);
		OpenImage(path_to_image);
		Filter();
		Wallace();
		cout << "Сохранение изображения" << endl;
		ReadPath(temp);
		SaveImage(temp);
		ShowImage(temp);
		break;
	case 6:
		cout << "Зашумленное изображение." << endl;
		ReadPath(path_to_image);
		OpenImage(path_to_image);
		percent;
		cout << "Введите шум" << endl;
		cin >> percent;
		AddNoise(percent);
		Wallace();
		cout << "Сохранение изображения" << endl;
		ReadPath(temp);
		SaveImage(temp);
		ShowImage(temp);
		break;
	case 7:
		cout << "Исходное отфильтрованное изображение." << endl;
		ReadPath(path_to_image);
		OpenImage(path_to_image);
		cout << "Введите шум" << endl;
		cin >> percent;
		AddNoise(percent);
		Filter();
		Wallace();
		cout << "Сохранение изображения" << endl;
		ReadPath(temp);
		SaveImage(temp);
		ShowImage(temp);
		break;
	case 8:
		cout << "Исходное изображение." << endl;
		ReadPath(path_to_image);
		OpenImage(path_to_image);
		cout << "Input Border_Width";
		cin >> borderWidth;
		cout << "StartColor" << endl;
		endColor = inputColor();
		cout << "EndColor" << endl;
		startColor = inputColor();
		AddGradientBorder(borderWidth, startColor, endColor);
		cout << "Сохранение изображения" << endl;
		ReadPath(temp);
		SaveImage(temp);
		ShowImage(temp);
		break;
	default:
		cout << "Вы неправильно выбрали" << endl;
		break;
	}
	cout << "END!" << endl;
    return 0;
}



