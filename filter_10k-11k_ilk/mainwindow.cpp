#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <qcustomplot.h>

static QString File_name; // File location for saving data

static QString Sampling_Rate; // Sampling rate of signal string version to take user input

static QVector <double> all_data;

static int Fs; // Sampling rate of signal int version

static QVector <double> data_to_filter; // Raw data to send filters

static QVector <double> average_signal; // Average signal from last 4 peaks

static int counter_glob; // shows how many data have arrived from beginning of program

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listWidget_filter->addItem("Bandpass 10k - 11k Butterworth");
    ui->listWidget_filter->addItem("Bandpass 9k-10k Butteroworth");
    ui->listWidget_filter->addItem("Bandpass 12k-13k Butterworth");

}

void MainWindow::setupProgram()
{
    setupRealtime(ui->customplot_raw,ui->customplot_2,ui->customplot_changing);
}

void MainWindow::setupRealtime(QCustomPlot *customplot_raw, QCustomPlot *customplot_2, QCustomPlot *customplot_changing)
{
    const int sampling_rate = Sampling_Rate.toInt();
    Fs = sampling_rate;
    const double delay_time = ((1.0/Fs));
    qDebug () << delay_time << endl;
    qDebug () << Fs << endl;

    QCPItemText *textLabel = new QCPItemText(customplot_raw);
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.5, 0);
    textLabel->setText("Raw Data");

    customplot_raw->addGraph();
    customplot_raw->graph(0)->setPen(QPen(QColor(40, 110, 255)));

    customplot_raw->axisRect()->setupFullAxesBox();
    ui->customplot_raw->yAxis->setRange(0 ,50);

    QCPItemText *textLabel_2 = new QCPItemText(customplot_2);
    textLabel_2->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    textLabel_2->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel_2->position->setCoords(0.5, 0);
    textLabel_2->setText("Butterworth Band pass filter [10hz 20hz]");

    customplot_2->addGraph();
    customplot_2->graph(0)->setPen(QPen(QColor(1,1,1)));
    customplot_2->addGraph();
    customplot_2->graph(1)->setPen(QPen(QColor(255,0,0)));
    customplot_2->graph(1)->setScatterStyle(QCPScatterStyle::ssStar);
    ui->customplot_2->graph(1)->setLineStyle(QCPGraph::lsNone);

    QCPItemText *textLabel_5 = new QCPItemText(customplot_changing);
    textLabel_5->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    textLabel_5->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel_5->position->setCoords(0.5, 0);
    textLabel_5->setText("User choice filtered Signal");

    customplot_changing->addGraph();
    customplot_changing->graph(0)->setPen(QPen(QColor(40, 110, 255)));

    customplot_changing->axisRect()->setupFullAxesBox();

    connect(ui->customplot_raw->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customplot_raw->xAxis, SLOT(setRange(QCPRange)));
    connect(ui->customplot_raw->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customplot_raw->yAxis, SLOT(setRange(QCPRange)));

    connect(ui->customplot_2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customplot_2->xAxis, SLOT(setRange(QCPRange)));
    connect(ui->customplot_2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customplot_2->yAxis, SLOT(setRange(QCPRange)));

    connect(customplot_changing->xAxis, SIGNAL(rangeChanged(QCPRange)), customplot_changing->xAxis, SLOT(setRange(QCPRange)));
    connect(customplot_changing->yAxis, SIGNAL(rangeChanged(QCPRange)), customplot_changing->yAxis, SLOT(setRange(QCPRange)));


    QSharedPointer<QCPAxisTicker> count(new QCPAxisTicker);
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0);

}

void MainWindow::realtimeDataSlot()
{
    static QTime time(QTime::currentTime());
    QElapsedTimer timer;
    timer.start();
    static int counter = 0;
    static QVector <double> data_to_peaks;  // data to check if there is a peak
    static QVector <double> R_peaks_location; // To plot average shape, last 4 peak location are stored in R_peaks_location
    static QVector <double> data_to_moving_filter; //Data to send moving average filter
    static int counter_for_check_length = 0;  // Hold the information how many data are deleted from data_to_peak signal

    const int Buffer_size = 800; // constant input buffer size

    QVector <double> data_to_plot; // New coming data
    QVector <double> filtered_signal;
    QVector <double> filtered_signal_changed_by_list;
    QVector <double> bandpass_40;
    QVector <double> keys;  // Stores information about the number of incoming data for each cycle

    data_to_plot = Get_data_from_csv();

    for (int var = counter; var < data_to_plot.size() + counter; ++var)
    {
        keys.append(var);
        counter_glob++;
    }

    data_to_filter.append(data_to_plot);

    filtered_signal_changed_by_list = Check_Which_Filter(keys);

    ui->customplot_changing->graph(0)->addData(keys,filtered_signal_changed_by_list,true);


    CheckDataLength_filter(data_to_filter);
    CheckDataLength_peaks(data_to_peaks,counter_for_check_length);

    filtered_signal_changed_by_list = Check_Which_Filter(keys);

    ui->customplot_raw->graph(0)->addData(keys,data_to_plot,true);

    filtered_signal = ApplyFilt_bandpass_1(data_to_filter,keys);

    data_to_peaks.append(filtered_signal);

    ui->customplot_2->graph(0)->addData(keys,filtered_signal);

    ui->customplot_raw->graph(0)->rescaleValueAxis(false,true);
    ui->customplot_2->graph(0)->rescaleValueAxis(false,true);
    ui->customplot_changing->graph(0)->rescaleValueAxis(false,true);


    ui->customplot_raw->xAxis->setRange(keys.last(),(Fs / 4), Qt::AlignRight);
    ui->customplot_2->xAxis->setRange(keys.last(), (Fs),Qt::AlignRight);
    ui->customplot_changing->xAxis->setRange(keys.last(), (Fs),Qt::AlignRight);



   ui->customplot_raw->replot();
   ui->customplot_2->replot();
   ui->customplot_changing->replot();

    counter = counter + Buffer_size;
    qDebug() << timer.elapsed() << endl;

}

QVector <double> MainWindow::Get_data_from_csv()
{
    static int counter = 0;
    const int Buffer_size = 800;

    QVector <double> data_to_plot;

    if( (counter + Buffer_size) < all_data.size() )
    {
        for (int var = counter; var < (counter + Buffer_size); ++var)
        {
               data_to_plot.append(all_data[var]);
        }

        counter = counter + Buffer_size;
    }

    else
    {
        counter = 0;

        for (int var = counter; var < (counter + Buffer_size); ++var)
        {
               data_to_plot.append(all_data[var]);
        }

        counter = counter + Buffer_size;
    }


    return data_to_plot;
}

QVector <double> MainWindow::ApplyFilt_bandpass_10k_11k(QVector <double> data_to_filter,QVector <double> data_counter_vector)
{

    int length_of_signal = data_to_filter.size();
    int naxpy;
    int filtered_signal_tmp;
    QVector <double> filtered_signal(length_of_signal);
    QVector <double> filtered_signal_last(data_counter_vector.size());


    static const double dv0[7] = { 0.00048, -0.0,
      -0.0014, -0.0, 0.0014, -0.0,
      -0.00048 }; //b

    double as;

    static const double dv1[7] = { 1.0, 0.9, 2.69,
      1.7, 2.65, 0.75,
      0.71 }; //a

    //  cuttoff low frequency to get rid of baseline wander
    //  cuttoff frequency to discard high frequency noise
    //  cutt off based on fs
    //  order of 3 less processing


    for (int var = 0; var < length_of_signal; ++var)
    {
        if(length_of_signal - var < 7)
        {
            naxpy = length_of_signal - 1 - var;
        }
        else
        {
            naxpy = 6;

        }

        for (int dummy = 0;  dummy <= naxpy; dummy++)
        {
            filtered_signal_tmp = dummy + var;
            filtered_signal[filtered_signal_tmp] += data_to_filter[var] * dv0[dummy];
        }

        if (length_of_signal - 1 - var < 6)
        {
            naxpy = length_of_signal - 2 - var;
        }

        else
        {
            naxpy = 5;

        }

        as = -filtered_signal[var];

        for (int temp = 0; temp <= naxpy; temp++)
        {
            filtered_signal_tmp = (temp + var) + 1;
            filtered_signal[filtered_signal_tmp] += as * dv1[temp + 1];
        }

    }

    // Take only plotting data for filter
    for (int var = 0; var < data_counter_vector.size()  ; ++var)
    {
       filtered_signal_last[var] =  filtered_signal[filtered_signal.size() + var - data_counter_vector.size()];
    }

    return filtered_signal_last;

}

QVector <double> MainWindow::ApplyFilt_bandpass_12k_13k(QVector <double> data_to_filter,QVector <double> data_counter_vector)
{

    int length_of_signal = data_to_filter.size();
    int naxpy;
    int filtered_signal_tmp;
    QVector <double> filtered_signal(length_of_signal);
    QVector <double> filtered_signal_last(data_counter_vector.size());


    static const double dv0[7] = { 0.00072570, -0.0,
      -0.0021771, -0.0, 0.0021771, -0.0,
      -0.00072570 };

    double as;
    static const double dv1[7] = { 1.0, 2.7, 5.11,
      5.572, 4.583, 2.171,
      0.718};

    //  cuttoff low frequency to get rid of baseline wander
    //  cuttoff frequency to discard high frequency noise
    //  cutt off based on fs
    //  order of 3 less processing


    for (int var = 0; var < length_of_signal; ++var)
    {
        if(length_of_signal - var < 7)
        {
            naxpy = length_of_signal - 1 - var;
        }
        else
        {
            naxpy = 6;

        }

        for (int dummy = 0;  dummy <= naxpy; dummy++)
        {
            filtered_signal_tmp = dummy + var;
            filtered_signal[filtered_signal_tmp] += data_to_filter[var] * dv0[dummy];
        }

        if (length_of_signal - 1 - var < 6)
        {
            naxpy = length_of_signal - 2 - var;
        }

        else
        {
            naxpy = 5;

        }

        as = -filtered_signal[var];

        for (int temp = 0; temp <= naxpy; temp++)
        {
            filtered_signal_tmp = (temp + var) + 1;
            filtered_signal[filtered_signal_tmp] += as * dv1[temp + 1];
        }

    }

    // Take only plotting data for filter
    for (int var = 0; var < data_counter_vector.size()  ; ++var)
    {
       filtered_signal_last[var] =  filtered_signal[filtered_signal.size() + var - data_counter_vector.size()];
    }

    return filtered_signal_last;

}
QVector <double> MainWindow::ApplyFilt_bandpass_1(QVector <double> data_to_filter,QVector <double> data_counter_vector)
{
    //    f1 = 10;
    //    f2 = 20;
    //    Fs = 330;
    //    Wn=[f1 f2] * 2 / Fs ;
    //    N = 3;
    //    [b, a] = butter(N,Wn,'bandpass');

    int length_of_signal = data_to_filter.size();
    int naxpy;
    int filtered_signal_tmp;
    QVector <double> filtered_signal(length_of_signal);
    QVector <double> filtered_signal_last(data_counter_vector.size());


    static const double dv0[7] = { 0.00072570895160154868, -0.0,
      -0.0021771268548046456, -0.0, 0.0021771268548046456, -0.0,
      -0.00072570895160154868 }; // b

    double as;
    static const double dv1[7] = { 1.0, -5.4143040199969077, 12.409518868546026,
      -15.404517279178839, 10.922127801362153, -4.194632661109563,
      0.682124952012655 }; //a


    for (int var = 0; var < length_of_signal; ++var)
    {
        if(length_of_signal - var < 7)
        {
            naxpy = length_of_signal - 1 - var;
        }
        else
        {
            naxpy = 6;

        }

        for (int dummy = 0;  dummy <= naxpy; dummy++)
        {
            filtered_signal_tmp = dummy + var;
            filtered_signal[filtered_signal_tmp] += data_to_filter[var] * dv0[dummy];
        }

        if (length_of_signal - 1 - var < 6)
        {
            naxpy = length_of_signal - 2 - var;
        }

        else
        {
            naxpy = 5;

        }

        as = -filtered_signal[var];

        for (int temp = 0; temp <= naxpy; temp++)
        {
            filtered_signal_tmp = (temp + var) + 1;
            filtered_signal[filtered_signal_tmp] += as * dv1[temp + 1];
        }

    }

    // Take only plotting data for filter
    for (int var = 0; var < data_counter_vector.size()  ; ++var)
    {
       filtered_signal_last[var] =  filtered_signal[filtered_signal.size() + var - data_counter_vector.size()];
    }

    return filtered_signal_last;



}
QVector <double> MainWindow::ApplyFilt_bandpass_9k_10k(QVector <double> data_to_filter,QVector <double> data_counter_vector)
{

    int length_of_signal = data_to_filter.size();
    int naxpy;
    int filtered_signal_tmp;
    QVector <double> filtered_signal(length_of_signal);
    QVector <double> filtered_signal_last(data_counter_vector.size());


    static const double dv0[7] = { 0, 0,
      -0.0014, -0.0, 0.0014, -0.0,
      -0.0 };

    double as;
    static const double dv1[7] = { 1.0, 0, 2.67,
      0, 2.4, 0,
      0.718 };

    //  cuttoff low frequency to get rid of baseline wander
    //  cuttoff frequency to discard high frequency noise
    //  cutt off based on fs
    //  order of 3 less processing


    for (int var = 0; var < length_of_signal; ++var)
    {
        if(length_of_signal - var < 7)
        {
            naxpy = length_of_signal - 1 - var;
        }
        else
        {
            naxpy = 6;

        }

        for (int dummy = 0;  dummy <= naxpy; dummy++)
        {
            filtered_signal_tmp = dummy + var;
            filtered_signal[filtered_signal_tmp] += data_to_filter[var] * dv0[dummy];
        }

        if (length_of_signal - 1 - var < 6)
        {
            naxpy = length_of_signal - 2 - var;
        }

        else
        {
            naxpy = 5;

        }

        as = -filtered_signal[var];

        for (int temp = 0; temp <= naxpy; temp++)
        {
            filtered_signal_tmp = (temp + var) + 1;
            filtered_signal[filtered_signal_tmp] += as * dv1[temp + 1];
        }

    }

    // Take only plotting data for filter
    for (int var = 0; var < data_counter_vector.size()  ; ++var)
    {
       filtered_signal_last[var] =  filtered_signal[filtered_signal.size() + var - data_counter_vector.size()];
    }

    return filtered_signal_last;

}
void MainWindow::CheckDataLength_peaks(QVector <double> &data_to_filter, int &counter_for_length)
{
    int size_of_filter_data = data_to_filter.size();
    if(size_of_filter_data > (3 * Fs))
    {
        for (int var = 0; var < size_of_filter_data - (3 * Fs); ++var)
        {
           data_to_filter.removeFirst();
           counter_for_length++;
        }
    }

}

void MainWindow::on_Browse_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,"Open a csv file","C://");
    File_name = file_name;
}

void MainWindow::on_Read_clicked()
{
    Sampling_Rate = ui->sample_rate->text();

    QFile file(File_name);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "file cannot open";
    }

    QStringList wordList;
    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        wordList.append(line.split(',').first());
    }
    QVector <QString> data;
    QVector <double> data_double;
    data = wordList.toVector();

    for (int var = 0; var < data.size(); ++var)
    {
        all_data.append(data[var].toDouble());
    }

    setupProgram();
}

void MainWindow::CheckDataLength_filter(QVector <double> &data_to_filter)
{
    int size_of_filter_data = data_to_filter.size();
    if(size_of_filter_data > (Fs))
    {
        for (int var = 0; var < size_of_filter_data - (Fs); ++var)
        {
           data_to_filter.removeFirst();
        }
    }
}

QVector <double> MainWindow::Check_Which_Filter(QVector <double> keys)
{
    int current_row = ui->listWidget_filter->currentRow();
    QVector <double> filtered_signal;

    switch (current_row)
    {
    case -1:
        for (int var = 0; var < keys.size(); ++var)
        {
            filtered_signal.append(0);
        }
        break;


    case 0:
        filtered_signal = ApplyFilt_bandpass_10k_11k(data_to_filter,keys);
        break;

    case 1:
        filtered_signal = ApplyFilt_bandpass_9k_10k(data_to_filter,keys);
        break;

    case 2:
        filtered_signal = ApplyFilt_bandpass_12k_13k(data_to_filter,keys);
        break;

   }

    return filtered_signal;

}

MainWindow::~MainWindow()
{
    delete ui;
}
