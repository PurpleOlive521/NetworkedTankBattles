#include "widgetTypes.h"
#include "widget.h"

void WidgetTreeItem::AddChild(WidgetTreeItem* child)
{
	children.emplace_back(child);
}

int WidgetTreeItem::GetChildCount() const
{
	return (int)children.size();
}

WidgetTreeItem* WidgetTreeItem::GetChildByRow(int row)
{
	if (row < 0 || GetChildCount() <= row)
	{
		return nullptr;
	}

	return children.at(row);
}

int WidgetTreeItem::GetRow() const
{
	if (parent == nullptr)
	{
		return 0;
	}

	// Find ourselves in our parents children
	auto it = std::find_if(parent->children.cbegin(), parent->children.cend(),
		[this](const WidgetTreeItem* widgetItem)
		{
			return widgetItem == this;
		}
	);

	if (it == parent->children.cend())
	{
		assertNoEntry(); // We are not amongst our direct parents children!
		return -1;
	}

	return (int)std::distance(parent->children.cbegin(), it);
}

WidgetTreeItem* WidgetTreeItem::GetParent()
{
	return parent;
}

WidgetTreeRoot* WidgetTreeItem::GetRoot()
{
	if (root)
	{
		return root;
	}

	if (!parent)
	{
		return nullptr;
	}

	return parent->GetRoot();
}

void WidgetTreeRoot::OnChildAdded(Widget* addedWidget)
{
	if (addedWidget->GetRenderOrder() != DEFAULT_RENDER_ORDER)
	{
		customRenderOrder.insert({ addedWidget->GetRenderOrder(), addedWidget });
	}
}
