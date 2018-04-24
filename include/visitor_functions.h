/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 2017 Richard Dijk <microvirus.multiplying@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef VISITOR_FUNCTIONS_H_INCLUDED
#define VISITOR_FUNCTIONS_H_INCLUDED

#include <file_io_visitor.h>

#include <functional>


namespace FileIOVisitor
{
	inline constexpr bool is_reader_visitor(FileReaderVisitor* v)
	{
		return true;
	}
	inline constexpr bool is_reader_visitor(FileWriterVisitor* v)
	{
		return false;
	}

	template <typename FieldT, typename FileT, typename GetterT, typename SetterT>
	void visit_property(FileReaderVisitor* v, GetterT&& getter, SetterT&& setter)
	{
		FieldT field;
		visit<FileT>(v, &field);
		setter(field);
	}

	template <typename FieldT, typename FileT, typename GetterT, typename SetterT>
	void visit_property(FileWriterVisitor* v, GetterT&& getter, SetterT&& setter)
	{
		FieldT field = getter();
		visit<FileT>(v, &field);
	}

	// Helper to use the above two with member function getters.
	template <typename FieldT, typename FileT, typename Visitor, typename T, typename GetterT, typename SetterT>
	void visit_property(Visitor* v, T* object, GetterT&& getter, SetterT&& setter)
	{
		visit_property<FieldT, FileT>(v, std::bind(getter, object), setter);
	}

	// Helper to visit an enum property (assumed to be backed by an int) as an int32_t FileT.
	template <typename Visitor, typename EnumT>
	static void visit_enum(Visitor* v, EnumT* c)
	{
		visit_property<int, int32_t>(v, [c]() -> int {return *c;}, [&c](int value) {*c = static_cast<EnumT>(value);});
	}
}

#endif // !VISITOR_FUNCTIONS_H_INCLUDED
