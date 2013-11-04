#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Console/ConsoleView.h"
#include <QtWidgets/qmessagebox.h>
#include <gstreamermm.h>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent),
ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->verticalLayout->addWidget(&plugins_tree);

	add_workspace_canvas();

	setAcceptDrops(false);
}

void MainWindow::add_workspace_canvas()
{
	QHBoxLayout *frameLayout = new QHBoxLayout;
	QFrame *frame = new QFrame;

	frameLayout->addWidget(&canvas);

	ConsoleView* console = new ConsoleView();
	console->set_model(Gst::Pipeline::create()); // TODO model should be created in the other place
	frameLayout->addWidget(console);
	frame->setLayout(frameLayout);
	frame->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	ui->mainVerticalLayout->addWidget(frame);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::set_model(QAbstractItemModel* model)
{
	connect(ui->factoryFilterEdit, SIGNAL(textChanged(QString)),
			model, SLOT(setFilterFixedString(QString)));

	plugins_tree.setModel(model);
}

void MainWindow::on_actionAbout_triggered(bool checked)
{
	QMessageBox::about(this, "About GstCreator",
			"Author:\tMarcin Kolny\n"
			"E-mail:\tmarcin.kolny[at]gmail.com\n"
			"License:\tGPL");
}

