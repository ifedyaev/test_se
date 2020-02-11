#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setup_gs();
    /* mute button */
    constexpr bool is_enabled = false;
    ui->m_push_button_lrt->setEnabled(is_enabled);
    ui->m_push_button_export_image->setEnabled(is_enabled);
    /* path */
    m_path_save = QDir::homePath();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup_gs()
{
    m_screen = std::make_shared<QGraphicsScene>(this);
    ui->m_view->setScene(m_screen.get());
    return;
}

void MainWindow::convert_rgb_to_bw(QImage& input_image)
{
    /* check gray */
    if(input_image.isGrayscale()){ return; }

    /* convert to gray */
    const int32_t n_col = input_image.width();
    const int32_t n_row = input_image.height();

    int32_t c,r;
    /* loop convert rgb -> gray */
    for(c = 0; c < n_col; ++c){
        for(r = 0; r < n_row; ++r){
            const QRgb cur_rgb = input_image.pixel(c,r);
            const int32_t gray = qGray(cur_rgb);
            input_image.setPixel(c,r,qRgb(gray,gray,gray));
        }/* end row */
    }/* end col */
    return;
}

void MainWindow::on_m_push_button_load_image_clicked()
{
    const QString file_name = QFileDialog::getOpenFileName(this,
                                                           tr("Select Image"),
                                                           m_path_save,
                                                           tr("Image (*.png)"));
    m_path_save = QFileInfo(file_name).absolutePath();/* save path */

    if (file_name.size() == 0){ return; }

    QImage input_image(file_name);

    convert_rgb_to_bw(/* input_image = */input_image);

    m_screen->clear();
    m_screen->addPixmap(QPixmap::fromImage(input_image));

    /* mute button */
    constexpr bool is_enabled = true;
    ui->m_push_button_lrt->setEnabled(is_enabled);
    return;
}

void MainWindow::on_m_push_button_lrt_clicked()
{
    /* mute button */
    constexpr bool is_enabled = true;
    ui->m_push_button_export_image->setEnabled(is_enabled);
    return;
}

void MainWindow::on_m_push_button_export_image_clicked()
{
    const QString file_name = QFileDialog::getSaveFileName(this,
                                                           tr("Save File"),
                                                           QDir(m_path_save).filePath("test.png"),
                                                           tr("Images (*.png)")
                                                           );
    m_path_save = QFileInfo(file_name).absolutePath();/* save path */
    if(file_name.size() == 0){ return; }
    return;
}
