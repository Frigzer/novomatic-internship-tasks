#pragma once
#include <SFML/Graphics.hpp>
#include <cstddef>

namespace task2::VisualConfig {

// Grid settings for the background
inline constexpr float MinorGridStep = 50.0f;
inline constexpr float MajorGridStep = 200.0f;
inline constexpr float GridTolerance = 0.001f;

inline const sf::Color ColorMajorGrid{ 50, 54, 62 };
inline const sf::Color ColorMinorGrid{ 36, 40, 46 };
inline const sf::Color ColorBackground{ 24, 28, 33 };

// Node layout and appearance
inline constexpr float NodeOutlineThickness  = 2.0f;
inline constexpr float NodeHeaderHeight      = 26.0f;
inline constexpr float NodeShadowOffset      = 4.0f;
inline constexpr float NodeHorizontalPadding = 10.0f;
inline constexpr float NodeTitleTopMargin    = 6.0f;
inline constexpr float NodeIdBottomOffset    = 22.0f;
inline constexpr unsigned int NodeTitleSize  = 16;
inline constexpr unsigned int NodeIdSize     = 12;

inline const sf::Color ColorNodeBody{ 42, 47, 56 };
inline const sf::Color ColorNodeHeader{ 58, 66, 78 };
inline const sf::Color ColorNodeOutline{ 95, 170, 255 };
inline const sf::Color ColorNodeShadow{ 0, 0, 0, 80 };
inline const sf::Color ColorTextId{ 190, 190, 190 };

// Edge routing and geometry
inline constexpr float EdgeMinHorizontalOffset = 40.0f;
inline constexpr float EdgeDetourYOffset       = 80.0f;
inline constexpr float EdgeBackExitOffset      = 40.0f;
inline const sf::Color ColorEdge               = sf::Color( 210, 210, 210 );

// Arrowhead dimensions and style
inline constexpr float ArrowLength = 10.0f;
inline constexpr float ArrowWidth  = 5.0f;
inline const sf::Color ColorArrow{ 220, 220, 220 };

// Vertex count for different edge types
inline constexpr std::size_t PathPointsForward  = 4;
inline constexpr std::size_t PathPointsBackward = 6;

}  // namespace task2::VisualConfig
