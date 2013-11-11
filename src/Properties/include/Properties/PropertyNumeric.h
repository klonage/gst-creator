/*
 * PropertyNumeric.h
 *
 *  Created on: 9 lis 2013
 *      Author: Marcin Kolny
 */

#ifndef PROPERTYNUMERIC_H_
#define PROPERTYNUMERIC_H_

#include "Property.h"
#include "utils/StringUtils.h"

template<typename T>
class PropertyNumeric : public Property
{
private:
	T value;

	QLineEdit* value_edit;

protected:
	virtual void build_widget();
	virtual void init();
public:
	PropertyNumeric(GParamSpec* param_spec,
			const Glib::RefPtr<Gst::Element>& element,
			T value);
	void set_value();
	void update_value()
	{
		set_value();
		init();
	}

	void get_min_max(T& min, T& max);
};

template<typename T>
PropertyNumeric<T>::PropertyNumeric(GParamSpec* param_spec,
		const Glib::RefPtr<Gst::Element>& element, T value)
		: Property(param_spec, element),
		  value(value),
		  value_edit(nullptr)
		{}

template<typename T>
void PropertyNumeric<T>::set_value()
{
	element->property(param_spec->name, value);
}

template<typename T>
void PropertyNumeric<T>::build_widget()
{
	T min, max;
	get_min_max(min, max);
	std::string min_max_info = "<" + std::to_string(min) + ", " + std::to_string(max) + ">";
	widget->layout()->addWidget(new QLabel(min_max_info.c_str()));
	value_edit = new QLineEdit();
	widget->layout()->addWidget(value_edit);
	QObject::connect(value_edit, &QLineEdit::textChanged, [&](const QString& txt){
		value = StringUtils::str_to_numeric<T>(txt.toUtf8().constData());
	});
	QObject::connect(value_edit, &QLineEdit::returnPressed, this, &PropertyNumeric<T>::update_value);
}

template<typename T>
void PropertyNumeric<T>::init()
{
	element->get_property(param_spec->name, value);
	value_edit->setText(std::to_string(value).c_str());
}

#define GET_MIN_MAX_FUNC(TYPE, Type, type) \
template<> \
void PropertyNumeric<type>::get_min_max(type& min, type& max) \
{ \
	GParamSpec##Type *pvalue = G_PARAM_SPEC_##TYPE(param_spec); \
	min = pvalue->minimum; \
	max = pvalue->maximum; \
} \

GET_MIN_MAX_FUNC(DOUBLE, Double, double)
GET_MIN_MAX_FUNC(FLOAT, Float, float)
GET_MIN_MAX_FUNC(ULONG, ULong, unsigned long)
GET_MIN_MAX_FUNC(LONG, Long, long)
GET_MIN_MAX_FUNC(INT, Int, int)
GET_MIN_MAX_FUNC(UINT, UInt, unsigned int)
GET_MIN_MAX_FUNC(INT64, Int64, gint64)
GET_MIN_MAX_FUNC(UINT64, UInt64, guint64)

#endif /* PROPERTYNUMERIC_H_ */
