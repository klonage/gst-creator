/*
 * ConsoleView.cpp
 *
 *  Created on: 28 pa? 2013
 *      Author: Marcin Kolny
 */

#include "ConsoleView.h"

ConsoleView::ConsoleView(QWidget* parent)
: QWidget(parent),
  parser(nullptr)
{
	edit = new QLineEdit();
	button = new QPushButton("Execute");

	QHBoxLayout* lay = new QHBoxLayout();

	lay->addWidget(edit);
	lay->addWidget(button);

	QObject::connect(button, &QPushButton::clicked, this, &ConsoleView::execute_command);
	QObject::connect(edit, &QLineEdit::returnPressed, this, &ConsoleView::execute_command);

	setLayout(lay);
}

ConsoleView::~ConsoleView()
{
	delete parser;
}

void ConsoleView::execute_command()
{
	Command* cmd = parser->parse(edit->text().toUtf8().constData());
	cmd->run_command();
	edit->clear();
}

void ConsoleView::set_model(const Glib::RefPtr<Gst::Pipeline>& model)
{
	delete parser;
	this->model = model;
	parser = new CommandParser(model);
}
