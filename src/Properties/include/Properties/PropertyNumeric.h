/*
 * PropertyNumeric.h
 *
 * Plik zawiera deklarację i definicję klasy PropertyNumeric
 *
 *  Created on: 9 lis 2013
 *      Author: Marcin Kolny
 */

#ifndef PROPERTYNUMERIC_H_
#define PROPERTYNUMERIC_H_

#include "Property.h"
#include "utils/StringUtils.h"

/**
 * Klasa reprezentuje właściwości typu numerycznego
 */
template<typename T>
class PropertyNumeric : public Property
{
private:
	/** Wartość właściwości */
	T value;
	/** widget służący do zmiany wartości */
	QLineEdit* value_edit;

protected:
	/**
	 * Funkcja buduje widget
	 */
	virtual void build_widget();

	/**
	 * Funkcja inicjalizuje obiekt właściwości
	 */
	virtual void init();

	/**
	 * Funkcja odczytuje wartość właściwości
	 */
	void read_var();
public:
	/**
	 * Konstruktor, inicjalizuje pola klasy
	 *
	 * @param parm_spec obiekt specyfikacji parametru
	 * @param element element, do którego należy właściwość
	 * @param value nowa wartość dla właściwości
	 */
	PropertyNumeric(GParamSpec* param_spec,
			const Glib::RefPtr<Gst::Element>& element,
			T value);
	/**
	 * Konstruktor, inicjalizuje pola klasy
	 *
	 * @param parm_spec obiekt specyfikacji parametru
	 * @param element element, do którego należy właściwość
	 */
	PropertyNumeric(GParamSpec* param_spec,
			const Glib::RefPtr<Gst::Element>& element);

	/**
	 * Ustawia wcześniej zadaną wartość właściwości
	 */
	void set_value();

	/**
	 * Zwraca aktualną wartosć właściwości jako obiekt typu string
	 *
	 * @returns wartość właściwości jako obiekt typu string
	 */
	std::string get_str_value() const;

	/**
	 * Aktualizuje zapisuje wartość i aktualizuje widget
	 */
	void update_value()
	{
		set_value();
		init();
	}

	/**
	 * Sprawdza, czy aktualna wartość właściwości, jest wartością domyślną
	 *
	 * @returns true, jeśli aktualna wartość jest wartością domyślną
	 */
	bool is_default_value() const;

	/**
	 * Zwraca nazwę typu właściwości
	 *
	 * @returns nazwę type właściwości
	 */
	std::string get_type_name() const;

	/**
	 * Odczytuje wartość minimalną i maksymalną dla danej właściwości
	 *
	 * @param min wartość minimalna
	 * @param max wartość maksymalna
	 */
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
PropertyNumeric<T>::PropertyNumeric(GParamSpec* param_spec,
		const Glib::RefPtr<Gst::Element>& element)
		: Property(param_spec, element),
		  value_edit(nullptr)
		  {
	read_var();
		  }

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
	read_var();
	value_edit->setText(std::to_string(value).c_str());
}

template<typename T>
void PropertyNumeric<T>::read_var()
{
	element->get_property(param_spec->name, value);
}

template<typename T>
std::string PropertyNumeric<T>::get_str_value() const
{
	return std::to_string(value);
}

#define GET_MIN_MAX_FUNC(TYPE, Type, type) \
		template<> \
		void PropertyNumeric<type>::get_min_max(type& min, type& max) \
		{ \
	GParamSpec##Type *pvalue = G_PARAM_SPEC_##TYPE(param_spec); \
	min = pvalue->minimum; \
	max = pvalue->maximum; \
		} \


#define UTILS_PROPERTY_NUM_FUNCS2(type1, type2) \
		template<> \
		bool PropertyNumeric<type2>::is_default_value() const \
		{ \
			const GValue* v = g_param_spec_get_default_value(param_spec); \
			type2 n = g_value_get_##type1(v); \
			return n == value; \
		} \
		template<> \
		std::string PropertyNumeric<type2>::get_type_name() const \
		{ \
			return #type2; \
		}

#define UTILS_PROPERTY_NUM_FUNCS(type) UTILS_PROPERTY_NUM_FUNCS2(type, type)

GET_MIN_MAX_FUNC(DOUBLE, Double, double)
UTILS_PROPERTY_NUM_FUNCS(double)
GET_MIN_MAX_FUNC(FLOAT, Float, float)
UTILS_PROPERTY_NUM_FUNCS(float)
GET_MIN_MAX_FUNC(INT, Int, int)
UTILS_PROPERTY_NUM_FUNCS(int)
GET_MIN_MAX_FUNC(UINT, UInt, unsigned int)
UTILS_PROPERTY_NUM_FUNCS(uint)
GET_MIN_MAX_FUNC(INT64, Int64, gint64)
UTILS_PROPERTY_NUM_FUNCS2(int64, gint64)
GET_MIN_MAX_FUNC(UINT64, UInt64, guint64)
UTILS_PROPERTY_NUM_FUNCS2(uint64, guint64)
#if !(__x86_64__ || __ppc64__)
GET_MIN_MAX_FUNC(ULONG, ULong, unsigned long)
UTILS_PROPERTY_NUM_FUNCS(ulong)
GET_MIN_MAX_FUNC(LONG, Long, long)
UTILS_PROPERTY_NUM_FUNCS(long)
#endif
#endif /* PROPERTYNUMERIC_H_ */
