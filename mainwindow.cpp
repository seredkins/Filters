#include "mainwindow.h"
#include <QImage>
#include <QPixmap>
#include <QImageReader>
#include <QFileDialog>
#include <math.h>


// Init and Deinit
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent) {
    draw_widget = new QLabel("", this);

    hlayout = new QHBoxLayout();
    open_button = new QPushButton("Open Image", this);
    hlayout->addWidget(open_button);
    for (int i = 0; i < BUTTONS_COUNT; ++i) {
        buttons[i] = new QPushButton("Button " + QString::number(i + 1), this);
        hlayout->addWidget(buttons[i]);
        buttons[i]->setEnabled(false);
    }

    vlayout = new QVBoxLayout();
    vlayout->addWidget(draw_widget);
    vlayout->addLayout(hlayout);

    QWidget *central = new QWidget(this);
    central->setLayout(vlayout);
    setCentralWidget(central);

    setFixedSize(800, 600);

    //initializing BOX by default
    box.set_size(3);
    for (int i = 0; i < box.size(); ++i)
            for (int j = 0; j < box.size(); ++j)
                box.box[i][j] = 0;
    //////////////////////////////

    sigma = 1; //initializing sigma for gaussian filter
    alpha = 1; //initializing alpha for sharpness

    connect(open_button, SIGNAL(clicked(bool)), this, SLOT(openImage()));
    connect(buttons[0], SIGNAL(clicked(bool)), this, SLOT(linearCorrection()));
    connect(buttons[1], SIGNAL(clicked(bool)), this, SLOT(colorCorrection()));
    connect(buttons[2], SIGNAL(clicked(bool)), this, SLOT(middleBlur()));
    connect(buttons[3], SIGNAL(clicked(bool)), this, SLOT(gaussianBlur()));
    connect(buttons[4], SIGNAL(clicked(bool)), this, SLOT(medialFilter()));
    connect(buttons[5], SIGNAL(clicked(bool)), this, SLOT(sharpness()));

    buttons[0]->setText("Linear Correction");
    buttons[1]->setText("Color Correction");
    buttons[2]->setText("Middle Blur");
    buttons[3]->setText("Gaussian Blur");
    buttons[4]->setText("Medial Filter");
    buttons[5]->setText("Sharpness");

}

MainWindow::~MainWindow(){
    delete open_button;
    delete draw_widget;
    for (int i = 0; i < BUTTONS_COUNT; ++i)
        delete buttons[i];
    delete hlayout;
    delete vlayout;
    if (image) delete image;
}


// Functions for working with image layout
void MainWindow::openImage() {
    QFileDialog dialog(this);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    dialog.setViewMode(QFileDialog::Detail);
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "C:/",
                                                    tr("Images (*.png *.xpm *.jpg)"));
    if (filename == "") image=nullptr;

    QImageReader reader(filename);
    reader.setAutoTransform(true);
    QImage newImage = reader.read();
    if (newImage.isNull()) image = nullptr;
    image = new QImage(newImage);
    updateImage();
    for (int i = 0; i < BUTTONS_COUNT; ++i)
        buttons[i]->setEnabled(true);
}

void MainWindow::updateImage() {
    if (!image) return;
    draw_widget->setPixmap(QPixmap::fromImage(*image));
}


// Functions for working with extended image
void MainWindow::initBuffImage(){
    shift = box.size() / 2;
    QImage _buffer_image(image->width() + 2*shift, image->height() + 2*shift, image->format());
    buffer_image = _buffer_image;

    for (int i = 0; i < buffer_image.width(); ++i)
        for (int j = 0; j < buffer_image.width(); ++j)
            if (!(i < shift || j < shift || i > buffer_image.width() - 1 - shift || j > buffer_image.height() - 1 - shift)) // if not edges
                buffer_image.setPixel(i, j, image->pixel(i - shift, j - shift)); //copying image pixels
}

void MainWindow::acceptBoxFilter(){

    for (int i = shift; i < buffer_image.width() - shift - 1; ++i)
        for (int j = shift; j < buffer_image.height() - shift - 1; ++j){
            double red = 0, green = 0, blue = 0;

            for (int m = 0; m < box.size(); ++m)
                for (int k = 0; k < box.size(); ++k){

                        red += qRed(buffer_image.pixel(i + m - shift, j + k - shift))*box.box[m][k];
                        green += qGreen(buffer_image.pixel(i + m - shift, j + k - shift))*box.box[m][k];
                        blue += qBlue(buffer_image.pixel(i + m - shift, j + k - shift))*box.box[m][k];

                }

            buffer_image.setPixel(i, j, qRgb(red, green, blue));

        }
}

void MainWindow::extractResult(){
    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j)
            image->setPixel(i, j, buffer_image.pixel(i + shift, j + shift));
}


// Brightness and Color borders for Correction
void MainWindow::getBrightnessBorders(){
    y_min = qGray(255, 255, 255);
    y_max = qGray(0, 0, 0);

//    y_min = wight.lightness();
//    y_min = black.lightness();

    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j){

//            QColor pix = image->pixel(i, j);
            int pix_bright = qGray(image->pixel(i, j));

            if (pix_bright > y_max)
                y_max = pix_bright;

            if (pix_bright < y_min)
                y_min = pix_bright;
        }
}

void MainWindow::getColorBorders(){

    red_max = 0;
    green_max = 0;
    blue_max = 0;

    red_min = 255;
    green_min = 255;
    blue_min = 255;

    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j){

            if (qRed(image->pixel(i, j)) > red_max)
                red_max = qRed(image->pixel(i, j));
            if (qRed(image->pixel(i, j)) < red_min)
                red_min = qRed(image->pixel(i, j));

            if (qGreen(image->pixel(i, j)) > green_max)
                green_max = qGreen(image->pixel(i, j));
            if (qGreen(image->pixel(i, j)) < green_min)
                green_min = qGreen(image->pixel(i, j));

            if (qBlue(image->pixel(i, j)) > blue_max)
                blue_max = qBlue(image->pixel(i, j));
            if (qBlue(image->pixel(i, j)) < blue_min)
                blue_min = qBlue(image->pixel(i, j));
        }
}


// Correctors
void MainWindow::linearCorrection() {
    getBrightnessBorders();

    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j){
            int red = qRed(image->pixel(i, j));
            int green = qGreen(image->pixel(i, j));
            int blue = qBlue(image->pixel(i, j));

            red = (red - y_min)*(255/(y_max - y_min));
            green = (green - y_min)*(255/(y_max - y_min));
            blue = (blue - y_min)*(255/(y_max - y_min));

            if (red > 255) red = 255;
            if (red < 0) red = 0;

            if (green > 255) green = 255;
            if (green < 0) green = 0;

            if (blue > 255) blue = 255;
            if (blue < 0) blue = 0;

            image->setPixel(i, j, qRgb(red, green, blue));
        }
    updateImage();
}

void MainWindow::colorCorrection() {
    getColorBorders();

    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j){
            int red, green, blue;

            red = (qRed(image->pixel(i, j)) - red_min)*(255 / (red_max - red_min));
            green = (qGreen(image->pixel(i, j)) - green_min)*(255 / (green_max - green_min));
            blue = (qBlue(image->pixel(i, j)) - blue_min)*(255 / (blue_max - blue_min));

            QRgb color = qRgb(red, green, blue);
            image->setPixel(i, j, color);
        }
    updateImage();
}


// Filters
void MainWindow::gaussianBlur() {
    box.set_size(5);
    initBuffImage();


    for (int i = 0; i < box.size(); ++i)
        for (int j = 0; j < box.size(); ++j)
            // Filling BoxFilter with normal distribution formula coefficents
            box.box[i][j] = pow(M_E, ( -((i - shift)*(i - shift) + (j - shift)*(j - shift)) / (2 * sigma * sigma)) ) /
                                        (2 * M_PI * sigma * sigma);

    acceptBoxFilter();
    extractResult();
    updateImage();
}

void MainWindow::middleBlur() {
    box.set_size(3);
    initBuffImage();

    for (int i = 0; i < box.size(); ++i)
            for (int j = 0; j < box.size(); ++j)
                this->box.box[i][j] = 0.11;

    acceptBoxFilter();
    extractResult();
    updateImage();
}

void MainWindow::medialFilter() {
    box.set_size(3);
    initBuffImage();

    //now working with buffer image that filed up by black on edges
    for (int i = shift; i < buffer_image.width() - shift; ++i)
        for (int j = shift; j < buffer_image.height() - shift; ++j){
            std::vector<QRgb> pixels;

            //adding neighboring pixels to the vector of pixels
            for (int m = 0; m < box.size(); ++m)
                for (int k = 0; k < box.size(); ++k)
                    pixels.push_back(buffer_image.pixel(i + m - shift, j + k - shift));


            std::sort(pixels.begin(), pixels.end()); //sorting pixels
            buffer_image.setPixel(i, j, pixels[4]); // taking mid pixel
        }

    extractResult(); //taking result from buffer image and putting it to image
    updateImage();
}

void MainWindow::sharpness(){
    QImage original(image->width(), image->height(), image->format());
    original = *image;
    gaussianBlur();

    QImage detailed(image->width(), image->height(), image->format());
    for (int i = 0; i < detailed.width(); ++i)
        for (int j = 0; j < detailed.height(); ++j){
            int red, green, blue;

            red = qRed(original.pixel(i, j)) - qRed(image->pixel(i, j));
            green = qGreen(original.pixel(i, j)) - qRed(image->pixel(i, j));
            blue = qBlue(original.pixel(i, j)) - qRed(image->pixel(i, j));

            if (red > 255) red = 255;
            if (red < 0) red = 0;

            if (green > 255) green = 255;
            if (green < 0) green = 0;

            if (blue > 255) blue = 255;
            if (blue < 0) blue = 0;

            detailed.setPixel(i, j, qRgb(red, green, blue));
        }

    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j){
            int red, green, blue;

            red = qRed(original.pixel(i, j)) + alpha * qRed(detailed.pixel(i, j));
            green = qGreen(original.pixel(i, j)) + alpha * qGreen(detailed.pixel(i, j));
            blue = qBlue(original.pixel(i, j)) + alpha * qBlue(detailed.pixel(i, j));

            if (red > 255) red = 255;
            if (red < 0) red = 0;

            if (green > 255) green = 255;
            if (green < 0) green = 0;

            if (blue > 255) blue = 255;
            if (blue < 0) blue = 0;

            image->setPixel(i, j, qRgb(red, green, blue));
    }
    updateImage();
}
