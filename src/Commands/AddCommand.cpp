/*
 * AddCommand.cpp
 *
 *  Created on: 2 lis 2013
 *      Author: Marcin Kolny
 */

#include "AddCommand.h"
#include "utils/EnumUtils.h"
#include "utils/GstUtils.h"
#include <set>

using namespace Gst;
using namespace std;
using Glib::RefPtr;

AddCommand::AddCommand(ObjectType type,
		const RefPtr<Element>& parent,
		const RefPtr<Object>& object)
: type(type),
  Command(CommandType::ADD),
  parent(parent),
  object(object)
{
}

AddCommand::~AddCommand()
{
}

void AddCommand::run_command(CommandListener* listener)
{
	run_command_ret(listener);
}

RefPtr<Object> AddCommand::run_command_ret(CommandListener* listener)
{
	if (type == ObjectType::PAD)
	{
		RefPtr<Pad> pad;
		if (GST_IS_PAD(object->gobj()))
		{
			pad = pad.cast_static(object);
			parent->add_pad(pad);
		}
		else if (GST_IS_PAD_TEMPLATE(object->gobj()))
		{
			pad = parent->request_pad(RefPtr<PadTemplate>::cast_static(object));
		}
		else
			throw runtime_error("cannot run command: object is not a pad");

		pad->signal_linked().connect([listener](const Glib::RefPtr<Gst::Pad>& pad) {
			if (listener != nullptr)
				listener->pad_linked(pad);
		});
		pad->signal_unlinked().connect([listener](const Glib::RefPtr<Gst::Pad>& pad) {
			if (listener != nullptr)
				listener->pad_unlinked(pad);
		});

		return pad;
	}
	else
	{
		if (GST_IS_ELEMENT(object->gobj()) && GST_IS_BIN(parent->gobj()))
		{
			RefPtr<Pipeline> pipeline = pipeline.cast_static(parent);
			RefPtr<Element> element = element.cast_static(object);
			pipeline->add(element);
			element->signal_pad_added().connect([listener](const Glib::RefPtr<Gst::Pad>& pad) {
				if (listener != nullptr)
					listener->pad_added(pad);

				pad->signal_linked().connect([listener](const Glib::RefPtr<Gst::Pad>& pad) {
					if (listener != nullptr)
						listener->pad_linked(pad);
				});
				pad->signal_unlinked().connect([&pad, listener](const Glib::RefPtr<Gst::Pad>& sec_pad) {
					if (listener != nullptr)
						listener->pad_unlinked(pad);
				});
			});
			element->signal_pad_removed().connect([listener](const Glib::RefPtr<Gst::Pad>& pad) {
				if (listener != nullptr)
					listener->pad_removed(pad);
			});

			auto iterator = element->iterate_pads();
			while (iterator.next())
			{
				iterator->signal_linked().connect([&iterator, listener](const Glib::RefPtr<Gst::Pad>& sec_pad) {
					if (listener != nullptr)
						listener->pad_linked(sec_pad);
				});
				iterator->signal_unlinked().connect([&iterator, listener](const Glib::RefPtr<Gst::Pad>& sec_pad) {
					if (listener != nullptr)
						listener->pad_unlinked(sec_pad);
				});
			}
			return element;
		}
		else
			throw runtime_error("cannot run command: invalid parent or object type");
	}
}

AddCommand* AddCommand::from_args(const vector<string>& args, const RefPtr<Pipeline>& model)
{
	if (args.size() < 1)
		syntax_error("invalid arguments count. Expected 1, but 0 found.");

	ObjectType type = EnumUtils<ObjectType>::string_to_enum(args[0]);

	return (type == ObjectType::ELEMENT) ?
			generate_add_element_command(args, model) :
			generate_add_pad_command(args, model);
}

AddCommand* AddCommand::generate_add_element_command(const vector<string>& args, const RefPtr<Pipeline>& model)
{
	set<int> allowed_args_count = {2, 3, 4, 5};

	if (allowed_args_count.find(args.size()) == allowed_args_count.end())
	{
		syntax_error("invalid arguments count. Expected 2, 3 or 5, but " +
				to_string(args.size()) + " found.");
	}

	RefPtr<Object> object = ElementFactory::create_element(args[1]);

	if (!object)
		throw runtime_error("Cannot find element " + args[1]);

	if (args.size() == 3 || args.size() == 5)
		object->set_name(args[2]);

	RefPtr<Element> parent;

	if (args.size() > 3)
	{
		if (!StringUtils::are_equal_case_no_sense(args[args.size()-2], "to"))
			syntax_error("expected `to`, but " + args[args.size()-2] + " found.");

		parent = GstUtils::find_element(args[args.size()-1], model);
	}
	else
		parent = model;

	return new AddCommand(ObjectType::ELEMENT, parent, object);
}

AddCommand* AddCommand::generate_add_pad_command(const vector<string>& args, const RefPtr<Pipeline>& model)
{
	if (args.size() != 5 && args.size() != 6)
		syntax_error("invalid arguments count. Expected 5 or 6, but " +
				to_string(args.size()) + " found.");

	if (!StringUtils::are_equal_case_no_sense(args[1], "to"))
		syntax_error("expected `using` but " + args[1] + " found.");

	if (!StringUtils::are_equal_case_no_sense(args[3], "using"))
		syntax_error("expected `to` but " + args[3] + " found.");

	RefPtr<Element> parent = GstUtils::find_element(args[2], model);
	RefPtr<PadTemplate> tpl = parent->get_pad_template(args[4]);

	if (!tpl)
		runtime_error("unknown pad template: " + args[2]);

	RefPtr<Object> object = Pad::create(tpl);

	if (args.size() == 6)
		object->set_name(args[5]);

	return new AddCommand(ObjectType::PAD, parent, object);
}

vector<string> AddCommand::get_suggestions(const vector<string>& args, const RefPtr<Pipeline>& model)
{
	try
	{
		ObjectType type;
		if (args.size() > 1)
			type = EnumUtils<ObjectType>::string_to_enum(args[0]);

		if (args.size() == 1)
			return EnumUtils<ObjectType>::get_string_values();
		else if (args.size() == 2 && type == ObjectType::ELEMENT)
			return GstUtils::get_avaliable_elements_string();
		else if (((args.size() == 3 || args.size() == 4) && type == ObjectType::ELEMENT
				&& !StringUtils::are_equal_case_no_sense(args[args.size()-2], "TO")) ||
				(args.size() == 2 && type == ObjectType::PAD))
			return {"TO"};
		else if (args.size() == 3 && type == ObjectType::PAD)
			return GstUtils::get_elements_from_bin_string(model, false);
		else if (args.size() == 4 && type == ObjectType::PAD)
			return {"USING"};
		else if (args.size() == 5 && type == ObjectType::PAD)
			return GstUtils::get_avaliable_pad_templates_string(GstUtils::find_element(args[2], model));
		else if ((args.size() == 4 || args.size() == 5) && type == ObjectType::ELEMENT)
			return GstUtils::get_elements_from_bin_string(model, true);
		else if ((args.size() == 5 || args.size() == 6) && type == ObjectType::PAD)
			return GstUtils::get_elements_from_bin_string(model, false);
	}
	catch (...) {}
	return vector<string>();
}
