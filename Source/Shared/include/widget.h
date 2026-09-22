#pragma once

#include "widgetTypes.h"
#include "raylib.h"

constexpr int DEFAULT_RENDER_ORDER = 0;

class Widget : public WidgetTreeItem
{
public:
	Widget() = default;

	void Update(float deltaTime);

	void Render() const;

	int GetRenderOrder() const;

protected:

	// If 0, then the widgets own order is used.
	int ZOrder = 0;

	AlignmentType alignment = AlignmentType::None;

	FillType fill = FillType::None;

	Vector2 margin = {};

	Vector2 position = {};
};