/*
 * GstBlock.h
 *
 *  Created on: 16 lis 2013
 *      Author: Marcin Kolny
 */

#ifndef GSTBLOCK_H_
#define GSTBLOCK_H_

#include <QtWidgets>
#include <gstreamermm.h>
#include <vector>
#include "GstPadWidget.h"

class GstBlockInfo;

class GstBlock : public QFrame
{
	Q_OBJECT

private:
	Glib::RefPtr<Gst::Element> model;

	QLayout* sink_layout;
	QLayout* src_layout;
	QLayout* main_layout;

	GstBlockInfo* info_parent;

	QLabel* name_label;

	std::vector<GstPadWidget*> pads;

	void update_pads();
	void add_pad(const Glib::RefPtr<Gst::Pad>& pad);
	void clear_element_view();
	void paintEvent(QPaintEvent * event);

	int get_sink_cnt();
	int get_src_cnt();

public:
	explicit GstBlock(const Glib::RefPtr<Gst::Element>& element_model, QWidget* parent = 0);
	virtual ~GstBlock(){}

	void update_element_view();
	GstPadWidget* find_pad(QPoint pt);
	GstPadWidget* find_pad(const std::string& name);

	QPoint get_pad_point(GstPadWidget* pad);

	Glib::RefPtr<Gst::Element> get_model(){ return model; }

	int get_width() { return grab().width();}
	int get_height() { return grab().height();}

	void set_info_parent(GstBlockInfo* info_parent);
};

#endif /* GSTBLOCK_H_ */