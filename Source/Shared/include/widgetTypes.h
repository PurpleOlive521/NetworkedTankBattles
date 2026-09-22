#pragma once
#include "core.h"
#include <vector>
#include <map>

enum class AlignmentType : uint8
{
	None,		// Absolute position is used, or widgets 0,0 if none is specified.
	Horizontal,	// Centered horizontally.
	Vertical,	// Centered vertically.
	Centered,	// Absolute center.

	ENUM_MAX
};

enum class FillType : uint8
{
	None,			// Absolute width and height is used.
	FillHorizontal,	// Fills horizontally.
	FillVertical,	// Fills vertically.
	All,			// Takes up as much space as possible in all directions.

	ENUM_MAX
};

class WidgetTreeRoot;

class WidgetTreeItem
{
public:
	WidgetTreeItem() = default;

	void AddChild(WidgetTreeItem* child);

	int GetChildCount() const;

	// Gets the child item in the specified row. Returns nullptr if none exists.
	WidgetTreeItem* GetChildByRow(int row);

	int GetRow() const;

	WidgetTreeItem* GetParent();

	// Recursively travel up the tree until the root is found. Returns nullptr if none is found.
	WidgetTreeRoot* GetRoot();

private:
	// TODO: Make use of unique ptrs
	std::vector<WidgetTreeItem*> children;

	WidgetTreeItem* parent = nullptr;

	// TODO: Weak ptr?
	WidgetTreeRoot* root = nullptr;
};

class Widget;

class WidgetTreeRoot
{
public:
	WidgetTreeRoot() = default;

	void OnChildAdded(Widget* addedWidget);

private:
	// Negatives are rendered before default tree items, while positives are rendered after the tree.
	std::multimap<int, Widget*> customRenderOrder;
};