#ifndef VISIT_DYN_ARRAY_H_INCLUDED
#define VISIT_DYN_ARRAY_H_INCLUDED

#include <ODYNARR.h>


template <typename Visitor>
static void visit_dyn_array(Visitor *v, DynArray *da)
{
	using namespace FileIOVisitor;

	visit<int32_t>(v, &da->ele_num);
	visit<int32_t>(v, &da->block_num);
	visit<int32_t>(v, &da->cur_pos);
	visit<int32_t>(v, &da->last_ele);
	visit<int32_t>(v, &da->ele_size);
	visit<int32_t>(v, &da->sort_offset);
	visit<int8_t>(v, &da->sort_type);
	v->skip(4); /* da->body_buf */
}

enum { DYN_ARRAY_RECORD_SIZE = 29 };

#endif // ! VISIT_DYN_ARRAY_H_INCLUDED
