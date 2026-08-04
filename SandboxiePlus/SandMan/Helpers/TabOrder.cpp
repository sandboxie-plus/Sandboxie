#include "stdafx.h"
#include "TabOrder.h"

// Items to be sorted by row & col
template <class T>
struct ChildItem
{
	T item;
	int row, col;
	template <class U>
	ChildItem(U&& item, int row, int col)
		: item(std::forward<U>(item)), row(row), col(col) {}
	bool operator<(const ChildItem& other) const  // make it sortable
	{
		if (row != other.row)
			return row < other.row;
		return col < other.col;  // sort first by row, then by col
	}
};

void GetWidgetsInOrder(QWidget* widget, std::vector<QWidget*>& widgets);
void GetWidgetsInOrder(QLayout* layout, std::vector<QWidget*>& widgets);

// Fills the given list with widgets in the correct tab order (recursively)
void GetWidgetsInOrder(QWidget* widget, std::vector<QWidget*>& widgets)
{
	if (widget->focusPolicy() != Qt::FocusPolicy::NoFocus)
	{
		widgets.push_back(widget);  // add the widget itself
	}

	if (QLayout* layout = widget->layout())
	{
		// if managed by a layout
		GetWidgetsInOrder(layout, widgets);
		return;
	}

	// If not managed by a layout, try to get its child widgets.
	// Here only children of QTabWidgets are actually processed.
	// More branches will be necessary if there's another type of container widget used.

	if (auto* parent = qobject_cast<QTabWidget*>(widget))
	{
		int cnt = parent->count();
		for (int i = 0; i < cnt; i++)
			GetWidgetsInOrder(parent->widget(i), widgets);
	}
}

// Fills the given list with widgets in the correct tab order (recursively)
void GetWidgetsInOrder(QLayout* layout, std::vector<QWidget*>& widgets)
{
	// Get a list of layout items in the layout,
	// then sort by rows and columns to get the correct tab order

	int cnt = layout->count();
	std::vector<ChildItem<QLayoutItem*>> items;

	if (QGridLayout* gridLayout = qobject_cast<QGridLayout*>(layout))
	{
		for (int i = 0; i < cnt; i++)
		{
			int row, col, rowSpan, colSpan;
			gridLayout->getItemPosition(i, &row, &col, &rowSpan, &colSpan);
			items.emplace_back(gridLayout->itemAt(i), row, col);
		}
	}
	else if (QFormLayout* formLayout = qobject_cast<QFormLayout*>(layout))
	{
		for (int i = 0; i < cnt; i++)
		{
			int row;
			QFormLayout::ItemRole role;
			formLayout->getItemPosition(i, &row, &role);
			items.emplace_back(formLayout->itemAt(i), row, (int)role);
		}
	}
	else
	{
		// For other types of layouts, preserve the order in the layout
		for (int i = 0; i < cnt; i++)
		{
			items.emplace_back(layout->itemAt(i), 0, i);
		}
	}

	std::stable_sort(items.begin(), items.end());

	// process all child layouts/widgets in the sorted order
	for (const auto& item : items)
	{
		if (QLayout* l = item.item->layout())
			GetWidgetsInOrder(l, widgets);
		else if (QWidget* w = item.item->widget())
			GetWidgetsInOrder(w, widgets);
	}
}

void SetTabOrder(QWidget* root)
{
	std::vector<QWidget*> widgets;
	GetWidgetsInOrder(root, widgets);
	QWidget* prev = nullptr;
	for (QWidget* widget : widgets)
	{
		if (prev)
			QWidget::setTabOrder(prev, widget);
		prev = widget;
	}
}

void SetTabOrder(QLayout* root)
{
	std::vector<QWidget*> widgets;
	GetWidgetsInOrder(root, widgets);
	QWidget* prev = nullptr;
	for (QWidget* widget : widgets)
	{
		if (prev)
			QWidget::setTabOrder(prev, widget);
		prev = widget;
	}
}
