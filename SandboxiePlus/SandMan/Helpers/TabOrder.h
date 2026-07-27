#pragma once

/**
 * @brief Sets the tab order for widgets under the given root widget.
 * @param root The root QWidget whose children will be ordered.
 */
void SetTabOrder(QWidget* root);

/**
 * @brief Sets the tab order for widgets in the given layout.
 * @param root The root QLayout containing the widgets to order.
 */
void SetTabOrder(QLayout* root);
