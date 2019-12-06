#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include "qcustomplot.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void setupProgram();

    void setupRealtime(QCustomPlot *customplot_raw, QCustomPlot *customplot_2, QCustomPlot *customplot_changing);

    ~MainWindow();

private slots:
    void on_Browse_clicked();

    void on_Read_clicked();

    void realtimeDataSlot();

private:

    Ui::MainWindow *ui;

    QVector <double>  Get_data_from_csv();

    void CheckDataLength_filter(QVector <double> &data_to_filter);

    QVector <double> ApplyFilt_bandpass_10k_11k(QVector <double> data_to_filter,QVector <double> data_counter_vector);

    QVector <double> ApplyFilt_bandpass_9k_10k(QVector <double> data_to_filter,QVector <double> data_counter_vector);

    QVector <double> ApplyFilt_bandpass_12k_13k(QVector <double> data_to_filter,QVector <double> data_counter_vector);

    QVector <double> ApplyFilt_bandpass_1(QVector <double> data_to_filter,QVector <double> data_counter_vector);

    void CheckDataLength_peaks(QVector <double> &data_to_filter, int &counter_for_check_length);


    QVector <double> Check_Which_Filter(QVector <double> keys);

    QTimer dataTimer;


};
#endif // MAINWINDOW_H
