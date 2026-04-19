#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

namespace task2 {

class ImGuiSfmlContext {
public:
	explicit ImGuiSfmlContext( sf::RenderWindow& window );
	~ImGuiSfmlContext() noexcept;

	ImGuiSfmlContext( const ImGuiSfmlContext& )            = delete;
	ImGuiSfmlContext& operator=( const ImGuiSfmlContext& ) = delete;
	ImGuiSfmlContext( ImGuiSfmlContext&& )                 = delete;
	ImGuiSfmlContext& operator=( ImGuiSfmlContext&& )      = delete;

	static void processEvent( sf::RenderWindow& window, const sf::Event& event );
	static void update( sf::RenderWindow& window, sf::Time deltaTime );
	static void render( sf::RenderWindow& window );

private:
	bool initialized_{ false };
};

}  // namespace task2
