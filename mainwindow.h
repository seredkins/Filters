#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>

#include "boxfilter.h"

namespace Ui {
    class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private:
    static const int BUTTONS_COUNT = 11;
    QLabel* draw_widget;
    QHBoxLayout *hlayout;
    QVBoxLayout *vlayout;
    QPushButton *open_button;
    QPushButton *buttons[BUTTONS_COUNT];
    QImage* image;
    QImage buffer_image;  // Extended image
    BoxFilter box;

    int y_max, y_min;
    int red_max, green_max, blue_max;
    int red_min, green_min, blue_min;

    int shift;            // Value for working with extended image (equals BoxFilter size / 2)
    double sigma, alpha;  // Coefficents for gaussianBlur and sharpness

    void getBrightnessBorders();  // Finds maximum and minimum brightness
    void getColorBorders();       // Finds maximum and minimum color

    // Functions for working with extended image
    // Needs because of BoxFilter
    // You cannot accept it to edge pixels because it will call not existed piexls then
    void initBuffImage();    // Inits extended to double BoxFilter's size image
    void acceptBoxFilter();  // Accepts BoxFilter to extended image
    void extractResult();    // Extracts normal size image from extended

private slots:
    void openImage();   // Open image from your computer
    void updateImage(); // Updates image at the layout

    // Correctors
    void linearCorrection();
    void colorCorrection();

    // Filters
    void gaussianBlur();
    void middleBlur();
    void medialFilter();
    void sharpness();
    void toGray();

    // Convertors
    void rotate90();
    void rotate270();
    void swapHorizontal();
    void swapVertical();
};

#endif // MAINWINDOW_H
