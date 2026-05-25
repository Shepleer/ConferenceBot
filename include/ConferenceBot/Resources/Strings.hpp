#pragma once
#include <string_view>

namespace ConferenceBot::Strings {

inline constexpr std::string_view BotStartMessageText =
    "Привет!\n"
    "Вы попали в бот студии разработки корпоративного мерча SATOSHI merch 🌪️\n"
    "Для участников MAC 2026 мы подготовили несколько активностей,"
    "чтобы принять участие подпишитесь на наш <a href=\"https://t.me/SATOSHI_mrch\">tg-канал</a> и нажмите “Готово”.\n";

inline constexpr std::string_view SubscriptionButtonText = "Готово";

inline constexpr std::string_view WantParticipateButtonText =
    "Хочу участвовать!";

inline constexpr std::string_view BackButtonText = "Назад";

inline constexpr std::string_view YouSubcribedMessageText = "Подписка есть";

inline constexpr std::string_view YouNotSubcribedMessageText =
    "А подписаться… 👉👈";

inline constexpr std::string_view SubscriptionConfirmedMessageText =
    "Супер! Теперь вы можете\n"
    "1. Показать подписку нашему менеджеру и вытянуть печенье с "
    "бизнес-предсказанием ;)\n"
    "2. Поучаствовать в конкурсе концептуальных фраз для affiliate мерча и "
    "выиграть одно из изделий SATOSHI с собственным брендингом.\n";

inline constexpr std::string_view ContestDescriptionMessageText =
    "Приглашаем поучаствовать в конкурсе от SATOSHI merch :)\n\n"
    "Что нужно сделать?\n"
    "Придумать концептуальную фразу для <b>affiliate мерча</b>  и заполнить форму "
    "ниже.\n\n"
    "Какой приз?\n"
    "Команда SATOSHI выберет 5 самых креативных фраз и сделает 1 из 4-х "
    "айтемов (футболка, лонг, панама, шоппер) на выбор с их нанесением для "
    "авторов.";

inline constexpr std::string_view ThankYouMessageText =
    "Результаты будут размещены в нашем <a href=\"https://t.me/SATOSHI_mrch\">канале</a> 28.05 во второй половине дня,"
    "а с победителями конкурса мы свяжемся лично в Telegram для уточнения "
    "деталей.\n";

inline constexpr std::string_view FormEnterNamePromptText = "Введите ваше имя";
inline constexpr std::string_view FormEnterCompanyPromptText =
    "В какой компании вы работаете?";
inline constexpr std::string_view FormEnterCompanyPositionPromptText =
    "Чем вы занимаетесь в компании? :)";
inline constexpr std::string_view FormEnterPhrasePromptText =
    "Введите affiliate фразу";
inline constexpr std::string_view FormCompletedText = "Спасибо за участие!";

inline constexpr std::string_view RegistrationFormTemplate =
    "Имя: {}\n"
    "Название компании: {}\n"
    "Должность: {}\n"
    "Affiliate фраза: {}\n";

inline constexpr std::string_view RegistrationFormEmptyFieldPlacehplderText =
    "[Waiting]";

inline constexpr std::string_view FormOutdatedMessageText =
    "Эта форма устарела, используйте актуальную ниже";

} // namespace ConferenceBot::Strings