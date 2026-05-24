#pragma once

#include <ConferenceBot/Resources/Callbacks.hpp>
#include <ConferenceBot/Resources/Strings.hpp>

#include <tgbot/tgbot.h>

namespace ConferenceBot::Keyboards {

inline TgBot::InlineKeyboardMarkup::Ptr makeCheckSubscriptionKeyboard() {
  auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
  auto button = std::make_shared<TgBot::InlineKeyboardButton>();

  button->text = std::string(Strings::SubscriptionButtonText);
  button->callbackData = std::string(Callbacks::CheckSubscriptionCallbackData);

  keyboard->inlineKeyboard.push_back({button});
  return keyboard;
}

inline TgBot::InlineKeyboardMarkup::Ptr wantParticipateKeyboard() {
  auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
  auto button = std::make_shared<TgBot::InlineKeyboardButton>();

  button->text = std::string(Strings::WantParticipateButtonText);
  button->callbackData = std::string(Callbacks::WantParticipateCallbackData);

  keyboard->inlineKeyboard.push_back({button});

  return keyboard;
}

inline TgBot::InlineKeyboardMarkup::Ptr formBackKeyboard() {
  auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
  auto button = std::make_shared<TgBot::InlineKeyboardButton>();

  button->text = std::string(Strings::BackButtonText);
  button->callbackData = std::string(Callbacks::FormBackButtonCallbackData);

  keyboard->inlineKeyboard.push_back({button});

  return keyboard;
}

} // namespace ConferenceBot::Keyboards