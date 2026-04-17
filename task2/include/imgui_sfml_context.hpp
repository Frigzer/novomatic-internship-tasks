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

	void processEvent( sf::RenderWindow& window, const sf::Event& event ) const;
	void update( sf::RenderWindow& window, sf::Time deltaTime ) const;
	void render( sf::RenderWindow& window ) const;

private:
	bool initialized_{ false };
};

}  // namespace task2
