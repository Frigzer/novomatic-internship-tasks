#include "imgui_sfml_context.hpp"

#include <imgui-SFML.h>

#include <stdexcept>

namespace task2 {

ImGuiSfmlContext::ImGuiSfmlContext( sf::RenderWindow& window ) {
	if ( !ImGui::SFML::Init( window ) ) {
		throw std::runtime_error( "Failed to initialize ImGui-SFML" );
	}

	initialized_ = true;
}

ImGuiSfmlContext::~ImGuiSfmlContext() noexcept {
	if ( initialized_ ) {
		ImGui::SFML::Shutdown();
	}
}

void ImGuiSfmlContext::processEvent( sf::RenderWindow& window, const sf::Event& event ) const {
	ImGui::SFML::ProcessEvent( window, event );
}

void ImGuiSfmlContext::update( sf::RenderWindow& window, sf::Time deltaTime ) const {
	ImGui::SFML::Update( window, deltaTime );
}

void ImGuiSfmlContext::render( sf::RenderWindow& window ) const {
	ImGui::SFML::Render( window );
}

}  // namespace task2
