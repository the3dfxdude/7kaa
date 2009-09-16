#ifndef __OGRPSEL_H
#define __OGRPSEL_H

#define MAX_SELECT_GROUP_NUM	9

//------------ class GroupSelect ----------//
class GroupSelect
{
	public:

	public:
		GroupSelect();
		~GroupSelect();

		void	init();
		void	deinit();

		void	group_units(int groupNum);
		void	select_grouped_units(int groupNum);
};

extern GroupSelect group_select;

#endif