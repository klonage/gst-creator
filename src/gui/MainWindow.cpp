#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Console/ConsoleView.h"
#include "PluginWizardDialog.h"
#include "Logger/LoggerView.h"
#include "controller.h"
#include "CodeGeneratorDialog.h"
#include "ObjectInspector/ObjectInspectorModel.h"
#include "ObjectInspector/ObjectInspectorFilter.h"
#include <QtWidgets/qmessagebox.h>
#include <gstreamermm.h>

MainWindow::MainWindow(MainController* controller, QWidget *parent)
: QMainWindow(parent),
  controller(controller),
  ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->objectInspectorFrame->layout()->addWidget(&plugins_tree);

	add_workspace_canvas();

	ui->splitter->setStretchFactor(0, 1);
	ui->splitter->setStretchFactor(1, 2);
	setAcceptDrops(false);

	reload_plugins();

	QObject::connect(ui->propertiesToolButton, &QToolButton::clicked, [this]{
		if (selected_element)
			PropertyCommand(selected_element, "", "").run_command(workspace);
	});
}

void MainWindow::reload_plugins()
{
	ObjectInspectorFilter* filter = new ObjectInspectorFilter();
	filter->setSourceModel(new ObjectInspectorModel());

	connect(ui->factoryFilterEdit, SIGNAL(textChanged(QString)),
			filter, SLOT(setFilterFixedString(QString)));

	plugins_tree.setModel(filter);
}

void MainWindow::add_workspace_canvas()
{
	QVBoxLayout *frameLayout = new QVBoxLayout;
	QFrame* workspace_frame = new QFrame();
	workspace_frame->setLayout(new QGridLayout());
	QFrame *frame = new QFrame;
	workspace = new WorkspaceWidget(controller->get_model());
	ConsoleView* console = new ConsoleView(workspace);
	LoggerView* logger = new LoggerView();

	QSplitter* spl = new QSplitter();

	QObject::connect(console, &ConsoleView::command_added, logger, &LoggerView::add_log);
	console->set_model(controller->get_model());

	QObject::connect(workspace, &WorkspaceWidget::current_element_changed, this, &MainWindow::current_element_info);

	workspace_frame->layout()->addWidget(workspace);
	spl->setOrientation(Qt::Vertical);
	spl->addWidget(workspace_frame);
	spl->addWidget(frame);
	frame->setLayout(frameLayout);
	frameLayout->addWidget(console);
	frameLayout->addWidget(logger);
	ui->rightFrame->layout()->addWidget(spl);

	spl->setStretchFactor(0, 15);
	spl->setStretchFactor(1, 1);

	controller->get_model()->signal_element_added().connect([this](const Glib::RefPtr<Gst::Element>& e) {
		workspace->new_element_added(e);
	});

	controller->get_model()->signal_element_removed().connect([this](const Glib::RefPtr<Gst::Element>& e) {
		workspace->element_removed(e);
	});

	controller->get_model()->get_bus()->add_watch(sigc::mem_fun(*logger, &LoggerView::add_bus_log));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_actionPlugin_Wizzard_triggered(bool checked)
{
	PluginWizardDialog plugin_wizzard;
	plugin_wizzard.setWindowTitle("Plugin Wizzard");

	if (!plugin_wizzard.exec())
		return;

	PluginCodeGenerator generator(plugin_wizzard.get_plugin(),
			std::vector<FactoryInfo>{plugin_wizzard.get_factory()});
	generator.generate_code(plugin_wizzard.get_directory().toUtf8().constData());
}

void MainWindow::on_actionAbout_triggered(bool checked)
{
	QMessageBox::about(this, "About GstCreator",
			"Author:\tMarcin Kolny\n"
			"E-mail:\tmarcin.kolny[at]gmail.com\n"
			"License:\tGPL");
}

void MainWindow::on_actionSave_As_triggered(bool checked)
{
	try
	{
		QString filename = QFileDialog::getSaveFileName(this, "Save Project", QDir::currentPath(),
				"gst-creator files (*.gstc);;All files (*.*)", 0, QFileDialog::DontUseNativeDialog);

		if (filename.isNull())
			return;

		FileWriter(filename.toUtf8().constData(), controller->get_model(),
				std::bind(&WorkspaceWidget::get_block_location, workspace, std::placeholders::_1))
		.save_model();
	}
	catch (const std::exception& ex)
	{
		show_error_box(QString("Cannot save project: ") + ex.what());
	}
}

void MainWindow::on_actionLoad_triggered(bool checked)
{
	QString filename = QFileDialog::getOpenFileName(this, "Save Project", QDir::currentPath(),
			"gst-creator files (*.gstc);;All files (*.*)", 0, QFileDialog::DontUseNativeDialog);

	if (filename.isNull())
		return;

	FileLoader(filename.toUtf8().constData(), controller->get_model(),
			std::bind(&WorkspaceWidget::set_block_location, workspace,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
	.load_model(workspace);
}

void MainWindow::current_element_info(const Glib::RefPtr<Gst::Element>& element)
{
	ui->currentElementLabel->setText(element ? element->get_name().c_str() : " none");
	selected_element = element;
}

void MainWindow::on_actionCode_Generator_triggered(bool checked)
{
	CodeGeneratorDialog dlg;
	if (!dlg.exec())
		return;

	CodeGenerator generator(controller->get_model());

	try
	{
		generator.generate_code(dlg.get_file_name().toUtf8().constData());
	}
	catch (const std::exception& e)
	{
		show_error_box(QString("Cannot generate code: ") + e.what());
	}
}

void MainWindow::on_actionLoad_Plugin_triggered(bool checked)
{
	try
	{
		QString filename = QFileDialog::getOpenFileName(this, "Load Plugin", QDir::currentPath(),
				"Shared Libraries (*.so);;All files (*.*)", 0, QFileDialog::DontUseNativeDialog);

		if (filename.isNull())
			return;

		Glib::RefPtr<Gst::Registry> registry = Gst::Registry::get();
		registry->add_plugin(Gst::Plugin::load_file(filename.toUtf8().constData()));

		reload_plugins();
	}
	catch (const std::exception& ex)
	{
		show_error_box(QString("Cannot load plugin: ") + ex.what());
	}
}

void MainWindow::on_actionAdd_Plugin_Path_triggered(bool checked)
{
	try
	{
		QString filename = QFileDialog::getExistingDirectory(this, "Load Plugin Path",
				".", QFileDialog::DontUseNativeDialog);

		if (filename.isNull())
			return;

		Glib::RefPtr<Gst::Registry> registry = Gst::Registry::get();

		if (registry->scan_path(filename.toUtf8().constData()))
			reload_plugins();
	}
	catch (const std::exception& ex)
	{
		show_error_box(QString("Cannot add plugin path: ") + ex.what());
	}
}

void MainWindow::on_actionExit_triggered(bool checked)
{
	this->close();
}

void MainWindow::show_error_box(QString text)
{
	QMessageBox message_box;
	message_box.critical(0, "gst-creator error", text);
}
