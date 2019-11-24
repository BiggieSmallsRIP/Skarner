#include "../SDK/PluginSDK.h"
#include "../SDK/EventArgs.h"
#include "../SDK/EventHandler.h"

PLUGIN_API const char PLUGIN_PRINT_NAME[32] = "Skarner";
PLUGIN_API const char PLUGIN_PRINT_AUTHOR[32] = "Biggie";
PLUGIN_API ChampionId PLUGIN_TARGET_CHAMP = ChampionId::Skarner;

namespace Menu
{
	IMenu* MenuInstance = nullptr;

	namespace Combo
	{
		IMenuElement* Enabled = nullptr;
		IMenuElement* UseQ = nullptr;
		IMenuElement* UseW = nullptr;
		IMenuElement* UseE = nullptr;
		IMenuElement* UseR = nullptr;
	}

	namespace Harass
	{
		IMenuElement* Enabled = nullptr;
		IMenuElement* UseQ = nullptr;
		IMenuElement* UseW = nullptr;
		IMenuElement* UseE = nullptr;
		IMenuElement* MinMana = nullptr;
	}
	namespace LaneClear
	{
		IMenuElement* Enabled = nullptr;
		IMenuElement* UseQ = nullptr;
		IMenuElement* UseE = nullptr;
		IMenuElement* MinMana = nullptr;
		//	IMenuElement* MinMinions = nullptr;
	}
	namespace Killsteal
	{
		IMenuElement* Enabled = nullptr;
		IMenuElement* UseE = nullptr;
	}

	namespace Misc
	{
		IMenuElement* UseWflee = nullptr;
		IMenuElement* UseWslow = nullptr;
		IMenuElement* Edash = nullptr;
		IMenuElement* Rdanger = nullptr;
		IMenuElement* Rtower = nullptr;
		IMenuElement* Rflash = nullptr;
		IMenuElement* SemiManulUlt = nullptr;
	}

	namespace Drawings
	{
		IMenuElement* Toggle = nullptr;
		IMenuElement* DrawQRange = nullptr;
		IMenuElement* DrawERange = nullptr;
		IMenuElement* DrawRRange = nullptr;
		//IMenuElement* DrawRTarget = nullptr;
	}

	namespace Colors
	{
		IMenuElement* QColor = nullptr;
		IMenuElement* RColor = nullptr;
		IMenuElement* EColor = nullptr;
	}
}

namespace Spells
{
	std::shared_ptr<ISpell> Q = nullptr;
	std::shared_ptr<ISpell> W = nullptr;
	std::shared_ptr<ISpell> E = nullptr;
	std::shared_ptr<ISpell> R = nullptr;
}

// combo
void ComboLogic()
{
	if (Menu::Combo::Enabled->GetBool() && Menu::Combo::UseQ->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo) && Spells::Q->IsReady())

	{// after first hit , deal extra magic dmg ( need to add that part)
	//  Bonus Magic Damage:»
	//	33 / 36 / 39 / 42 / 45 % AD(+30 % AP)
		auto TargetQ = g_Common->GetTarget(Spells::Q->Range(), DamageType::Physical);
		if (TargetQ && TargetQ->IsValidTarget())
			Spells::Q->Cast();
	}

	if (Menu::Combo::Enabled->GetBool() && Menu::Combo::UseE->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo) && Spells::E->IsReady())
	{//if E leaves debuff on target, next auto deals bonus physical dmg ( need to add that part)
		//Bonus Physical Damage :
		//30 / 50 / 70 / 90 / 110
		auto TargetE = g_Common->GetTarget(Spells::E->Range(), DamageType::Magical);
		if (TargetE && TargetE->IsValidTarget())
			Spells::E->Cast(TargetE, HitChance::High);
	}

	if (Menu::Combo::Enabled->GetBool() && Menu::Combo::UseW->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo) && Spells::W->IsReady())
	{
		const auto Enemies = g_ObjectManager->GetChampions(false);
		for (auto Enemy : Enemies)
		{
			if (Enemy->HasBuff(hash("skarnerfractureslow")))
				Spells::W->Cast();
		}
		if (g_LocalPlayer->HasBuff(hash("skarnerimpalebuff")))
			Spells::W->Cast();
	}
}

// harass
void HarassLogic()
{
	{
		if (g_LocalPlayer->ManaPercent() < Menu::Harass::MinMana->GetInt())
			return;
	}
	if (Menu::Harass::Enabled->GetBool() && Menu::Harass::UseQ->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeHarass) && Spells::Q->IsReady())
	{
		auto Target = g_Common->GetTarget(Spells::Q->Range(), DamageType::Physical);
		if (Target && Target->IsValidTarget())
			Spells::Q->Cast();
	}
	if (Menu::Harass::Enabled->GetBool() && Menu::Harass::UseE->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeHarass) && Spells::E->IsReady())
	{//if E leaves debuff on target, next auto deals bonus physical dmg ( need to add that part)
		//Bonus Physical Damage :
		//30 / 50 / 70 / 90 / 110
		auto Target = g_Common->GetTarget(Spells::E->Range(), DamageType::Magical);
		if (Target && Target->IsValidTarget())
			Spells::E->Cast(Target, HitChance::High);
	}
	if (Menu::Harass::Enabled->GetBool() && Menu::Harass::UseW->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeHarass) && Spells::W->IsReady())
	{
		const auto Enemies = g_ObjectManager->GetChampions(false);
		for (auto Enemy : Enemies)
		{
			if (Enemy->HasBuff(hash("skarnerfractureslow")))
				Spells::W->Cast();
		}
	}
}

void FleeLogic()
{
	if (Menu::Misc::UseWflee->GetBool() && Spells::W->IsReady())
	{
		Spells::W->Cast();
	}
}
// killsteal
void KillstealLogic()
{
	const auto Enemies = g_ObjectManager->GetChampions(false);
	for (auto Enemy : Enemies)
	{
		if (Menu::Killsteal::UseE->GetBool() && Spells::E->IsReady() && Enemy->IsInRange(Spells::E->Range()))
		{
			auto EDamage = g_Common->GetSpellDamage(g_LocalPlayer, Enemy, SpellSlot::E, false);
			if (Enemy->IsValidTarget() && EDamage >= Enemy->RealHealth(false, true))
				Spells::E->Cast(Enemy);
		}
	}
}


// lane clear
void LaneCLearLogic()
{
	if (g_LocalPlayer->ManaPercent() < Menu::Harass::MinMana->GetInt())
		return;
	{
		auto Target = g_Orbwalker->GetTarget();
		if (Target && (Target->IsMinion() || Target->IsMonster()))

			if (Spells::Q->IsReady() && Menu::LaneClear::UseQ->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeLaneClear))
				Spells::Q->Cast();
	}
	{auto Monster = g_Orbwalker->GetTarget();
	if (Monster && (Monster->IsMinion() || Monster->IsMonster()))

		if (Spells::E->IsReady() && Menu::LaneClear::UseE->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeLaneClear))
			Spells::E->Cast(Monster, HitChance::High);
	}
}
void UltLogic()
{
	// Semi-Manual ult
	if (Menu::Combo::UseR->GetBool() && Menu::Misc::SemiManulUlt->GetBool() && Spells::R->IsReady())
	{
		
		auto TargetR = g_Common->GetTarget(Spells::R->Range(), DamageType::Physical);
		if (TargetR && TargetR->IsValidTarget())
			Spells::R->Cast(TargetR);

		if (Menu::Combo::Enabled->GetBool() && Menu::Combo::UseW->GetBool() && Spells::W->IsReady())
			Spells::W->Cast();

		auto TargetQ = g_Common->GetTarget(Spells::Q->Range(), DamageType::Physical);
		if (TargetQ && TargetQ->IsValidTarget())
			Spells::Q->Cast();

		auto TargetE = g_Common->GetTarget(Spells::E->Range(), DamageType::Magical);
		if (TargetE && TargetE->IsValidTarget())
			Spells::E->Cast(TargetE, HitChance::High);
	}
}
//dont auto in ult
void OnBeforeAttack(BeforeAttackOrbwalkerArgs* args)
{
	if (g_LocalPlayer->HasBuff(hash("skarnerimpalebuff")))
	{
		if (args->Target)
			args->Process = false;
	}
}

void OnGameUpdate()
{
	if (Menu::Combo::Enabled->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo))
		ComboLogic();

	if (Menu::Misc::SemiManulUlt->GetBool())
	{
		g_Orbwalker->MoveTo(g_Common->CursorPosition());
		UltLogic();
	}
	if (Menu::Harass::Enabled->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeHarass))
		HarassLogic();

	if (Menu::LaneClear::Enabled->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeLaneClear))
		LaneCLearLogic();

	if (Menu::Killsteal::Enabled->GetBool())
		KillstealLogic();

	if (Menu::Misc::UseWflee->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeFlee))
		FleeLogic();

	// E on Dash
	if (Menu::Misc::Edash->GetBool() && Spells::E->IsReady())
	{
		const auto Enemies = g_ObjectManager->GetChampions(false);
		for (auto Enemy : Enemies)
		{
			if (Enemy->IsDashing())
			{
				const auto DashData = Enemy->GetDashData();
				if (g_LocalPlayer->Distance(DashData.EndPosition) < 630.f)
					Spells::E->Cast(Enemy, HitChance::High);
			}
		}
	}
	// W on slow
	if (Menu::Misc::UseWslow->GetBool() && Spells::W->IsReady() && g_LocalPlayer->HasBuffOfType(BuffType::Slow))
		Spells::W->Cast();

	//
	if (Menu::Misc::Rtower->GetBool() && Spells::R->IsReady())
	{
		auto TargetR = g_Common->GetTarget(Spells::R->Range(), DamageType::Magical);
		if (TargetR && TargetR->IsAIHero() && TargetR->IsUnderAllyTurret())
			//if (TargetR && g_HealthPrediction->HasTurretAggro(TargetR))
			if (Menu::Misc::Rtower->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo))
				Spells::R->Cast(TargetR);
	}
}
//R on Dangerous
void OnProcessSpell(IGameObject* Owner, OnProcessSpellEventArgs* Args)
{
	if (Menu::Misc::Rdanger->GetBool() && Spells::R->IsReady() && g_LocalPlayer->Distance(350.f))
	{
		if (Owner->ChampionId() == ChampionId::Katarina && Args->SpellSlot == SpellSlot::R)
			Spells::R->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::MissFortune && Args->SpellSlot == SpellSlot::R)
			Spells::R->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Lucian && Args->SpellSlot == SpellSlot::R)
			Spells::R->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Tristana && Args->SpellSlot == SpellSlot::W)
			Spells::R->Cast(Owner);
		{
			
			auto Spellbook = Owner->GetSpellbook();
			auto flash_spellslot = Owner->GetSpellbook()->GetSpellSlotFromName("SummonerFlash");
			if (Menu::Misc::Rflash->GetBool() && Args->SpellSlot == flash_spellslot)
			Spells::R->Cast(Owner);
		}
		if (Owner->ChampionId() == ChampionId::Twitch && Args->SpellSlot == SpellSlot::R)
			Spells::R->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Ahri && Args->SpellSlot == SpellSlot::R)
			Spells::R->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::KogMaw && Args->SpellSlot == SpellSlot::W)
			Spells::R->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Zed && Args->SpellSlot == SpellSlot::R)
			Spells::R->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Talon && Args->SpellSlot == SpellSlot::R)
			Spells::R->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Akali && Args->SpellSlot == SpellSlot::R)
			Spells::R->Cast(Owner);
	}
}

// drawings
void OnHudDraw()
{
	if (!Menu::Drawings::Toggle->GetBool() || g_LocalPlayer->IsDead())
		return;

	const auto PlayerPosition = g_LocalPlayer->Position();
	const auto CirclesWidth = 1.5f;

	if (Menu::Drawings::DrawQRange->GetBool() && !Spells::Q->CooldownTime())
		g_Drawing->AddCircle(PlayerPosition, Spells::Q->Range(), Menu::Colors::QColor->GetColor(), CirclesWidth);

	if (Menu::Drawings::DrawERange->GetBool() && !Spells::E->CooldownTime())
		g_Drawing->AddCircle(PlayerPosition, Spells::E->Range(), Menu::Colors::EColor->GetColor(), CirclesWidth);

	if (Menu::Drawings::DrawRRange->GetBool() && !Spells::R->CooldownTime())
		g_Drawing->AddCircle(PlayerPosition, Spells::R->Range(), Menu::Colors::RColor->GetColor(), CirclesWidth);
}

PLUGIN_API bool OnLoadSDK(IPluginsSDK* plugin_sdk)
{
	DECLARE_GLOBALS(plugin_sdk);

	if (g_LocalPlayer->ChampionId() != ChampionId::Skarner)
		return false;

	using namespace Menu;
	using namespace Spells;

	MenuInstance = g_Menu->CreateMenu("Skarner", "Skarner by Biggie");

	const auto ComboSubMenu = MenuInstance->AddSubMenu("Combo", "combo_menu");
	Menu::Combo::Enabled = ComboSubMenu->AddCheckBox("Enable Combo", "enable_combo", true);
	Menu::Combo::UseQ = ComboSubMenu->AddCheckBox("Use Q", "combo_use_q", true);
	Menu::Combo::UseW = ComboSubMenu->AddCheckBox("Use W", "combo_use_w", true);
	Menu::Combo::UseE = ComboSubMenu->AddCheckBox("Use E", "combo_use_e", true);
	Menu::Combo::UseR = ComboSubMenu->AddCheckBox("Use R", "combo_use_r", true);

	const auto HarassSubMenu = MenuInstance->AddSubMenu("Harass", "harass_menu");
	Menu::Harass::Enabled = HarassSubMenu->AddCheckBox("Enable Harass", "enable_harass", true);
	Menu::Harass::UseQ = HarassSubMenu->AddCheckBox("Use Q", "harass_use_q", true);
	Menu::Harass::UseW = HarassSubMenu->AddCheckBox("Use W", "harass_use_w", true);
	Menu::Harass::UseE = HarassSubMenu->AddCheckBox("Use E", "harass_use_e", true);
	Menu::Harass::MinMana = HarassSubMenu->AddSlider("Min Mana", "min_mana_harass", 50, 0, 100, true);

	const auto LaneClearSubMenu = MenuInstance->AddSubMenu("Lane Clear", "laneclear_menu");
	Menu::LaneClear::Enabled = LaneClearSubMenu->AddCheckBox("Enable Lane Clear", "enable_laneclear", true);
	Menu::LaneClear::UseQ = LaneClearSubMenu->AddCheckBox("Use Q", "laneclear_use_q", false);
	Menu::LaneClear::UseE = LaneClearSubMenu->AddCheckBox("Use E", "laneclear_use_e", true);
	Menu::LaneClear::MinMana = LaneClearSubMenu->AddSlider("Min Mana", "min_mana_laneclear", 50, 0, 100, true);
	//	Menu::LaneClear::MinMinions = LaneClearSubMenu->AddSlider("Min minions", "lane_clear_min_minions", 3, 0, 9);

	const auto KSSubMenu = MenuInstance->AddSubMenu("KS", "ks_menu");
	Menu::Killsteal::Enabled = KSSubMenu->AddCheckBox("Enable Killsteal", "enable_ks", true);
	Menu::Killsteal::UseE = KSSubMenu->AddCheckBox("Use E", "e_ks", true);

	const auto MiscSubMenu = MenuInstance->AddSubMenu("Misc", "misc_menu");
	Menu::Misc::UseWflee = MiscSubMenu->AddCheckBox("Use W in Flee", "w_flee", true);
	Menu::Misc::UseWslow = MiscSubMenu->AddCheckBox("Auto W on Slows", "w_slow", true);
	Menu::Misc::Edash = MiscSubMenu->AddCheckBox("Auto E on Dash", "e_dash", true);
	Menu::Misc::Rtower = MiscSubMenu->AddCheckBox("Auto R under our Tower", "r_tower", false);
	Menu::Misc::Rdanger = MiscSubMenu->AddCheckBox("R Dangerous Abilties", "r_danger", true);
	Menu::Misc::Rflash = MiscSubMenu->AddCheckBox("Auto R on enemy flash", "r_flash", true);
	Menu::Misc::SemiManulUlt = MiscSubMenu->AddKeybind("Semi Manual R", "semi_manual_ult", 'T', false, KeybindType_Hold);

	const auto DrawingsSubMenu = MenuInstance->AddSubMenu("Drawings", "drawings_menu");
	Drawings::Toggle = DrawingsSubMenu->AddCheckBox("Enable Drawings", "drawings_toggle", true);
	Drawings::DrawQRange = DrawingsSubMenu->AddCheckBox("Draw Q Range", "draw_q", true);
	Drawings::DrawRRange = DrawingsSubMenu->AddCheckBox("Draw R Range", "draw_r", true);
	Drawings::DrawERange = DrawingsSubMenu->AddCheckBox("Draw E Range", "draw_e", true);

	const auto ColorsSubMenu = DrawingsSubMenu->AddSubMenu("Colors", "color_menu");
	Colors::QColor = ColorsSubMenu->AddColorPicker("Q Range", "color_q_range", 0, 175, 255, 180);
	Colors::RColor = ColorsSubMenu->AddColorPicker("R Range", "color_r_range", 200, 200, 200, 180);
	Colors::EColor = ColorsSubMenu->AddColorPicker("E Range", "color_e_range", 210, 210, 210, 180);

	Spells::Q = g_Common->AddSpell(SpellSlot::Q, 345.f);
	Spells::W = g_Common->AddSpell(SpellSlot::W);
	Spells::E = g_Common->AddSpell(SpellSlot::E, 900.f);
	Spells::R = g_Common->AddSpell(SpellSlot::R, 350.f);

	// pred hitchance is very good with these weird values
	Spells::E->SetSkillshot(0.5f, 70.f, 1500.f, kCollidesWithNothing && kCollidesWithYasuoWall, kSkillshotLine);

	EventHandler<Events::OnProcessSpellCast>::AddEventHandler(OnProcessSpell);
	//EventHandler<Events::OnBuff>::AddEventHandler(OnBuffChange);
	EventHandler<Events::GameUpdate>::AddEventHandler(OnGameUpdate);
	//EventHandler<Events::OnAfterAttackOrbwalker>::AddEventHandler(OnAfterAttack);
	EventHandler<Events::OnHudDraw>::AddEventHandler(OnHudDraw);
	EventHandler<Events::OnBeforeAttackOrbwalker>::AddEventHandler(OnBeforeAttack);

	g_Common->ChatPrint("<font color='#FFC300'>Skarner Loaded!</font>");
	g_Common->Log("Skarner plugin loaded.");

	return true;
}

PLUGIN_API void OnUnloadSDK()
{
	Menu::MenuInstance->Remove();

	EventHandler<Events::OnBeforeAttackOrbwalker>::RemoveEventHandler(OnBeforeAttack);
	EventHandler<Events::GameUpdate>::RemoveEventHandler(OnGameUpdate);
	EventHandler<Events::OnProcessSpellCast>::RemoveEventHandler(OnProcessSpell);
	EventHandler<Events::GameUpdate>::RemoveEventHandler(OnGameUpdate);
	//EventHandler<Events::OnAfterAttackOrbwalker>::RemoveEventHandler(OnAfterAttack);
	EventHandler<Events::OnHudDraw>::RemoveEventHandler(OnHudDraw);
	//	EventHandler<Events::OnBuff>::RemoveEventHandler(OnBuffChange);

	g_Common->ChatPrint("<font color='#00BFFF'>Skarner Unloaded.</font>");
}