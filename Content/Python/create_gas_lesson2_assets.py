import unreal


ROOT_PATH = "/Game/GASLesson2"
EFFECT_PATH = f"{ROOT_PATH}/Effects"
ATTRIBUTE_CLASS_PATH = "/Script/UENet.GASLesson2AttributeSet"


def log(message):
    unreal.log_warning(f"GASLESSON2_ASSET: {message}")


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def create_or_load_blueprint(asset_name, package_path, parent_class):
    asset_path = f"{package_path}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        existing_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        require(isinstance(existing_asset, unreal.Blueprint), f"{asset_path} exists but is not a Blueprint")
        log(f"Load existing {asset_path}")
        return existing_asset

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name,
        package_path,
        unreal.Blueprint,
        factory,
    )
    require(blueprint is not None, f"Failed to create {asset_path}")
    log(f"Created {asset_path}")
    return blueprint


def get_blueprint_cdo(blueprint):
    generated_class = blueprint.generated_class()
    require(generated_class is not None, f"{blueprint.get_path_name()} has no generated class")
    return unreal.get_default_object(generated_class)


def make_scalable_magnitude(template, value):
    magnitude = template.copy()
    magnitude.import_text(
        "(MagnitudeCalculationType=ScalableFloat,"
        f"ScalableFloatMagnitude=(Value={float(value):.6f}))"
    )
    return magnitude


def make_modifier(attribute_name, value=None, set_by_caller_tag=None):
    modifier = unreal.GameplayModifierInfo()
    if set_by_caller_tag:
        magnitude_text = (
            "(MagnitudeCalculationType=SetByCaller,"
            f'SetByCallerMagnitude=(DataTag=(TagName="{set_by_caller_tag}")))'
        )
    else:
        magnitude_text = (
            "(MagnitudeCalculationType=ScalableFloat,"
            f"ScalableFloatMagnitude=(Value={float(value):.6f}))"
        )

    modifier.import_text(
        "("
        f'Attribute=(AttributeName="{attribute_name}",'
        f"Attribute={ATTRIBUTE_CLASS_PATH}:{attribute_name},"
        f"AttributeOwner={ATTRIBUTE_CLASS_PATH}),"
        "ModifierOp=AddBase,"
        f"ModifierMagnitude={magnitude_text}"
        ")"
    )
    require(
        modifier.get_editor_property("attribute").get_editor_property("attribute_name") == attribute_name,
        f"Failed to resolve modifier attribute {attribute_name}",
    )
    return modifier


def make_gameplay_tags(tag_names):
    tags_text = ",".join(f'(TagName="{tag_name}")' for tag_name in tag_names)
    gameplay_tags = unreal.GameplayTagContainer()
    gameplay_tags.import_text(f"(GameplayTags=({tags_text}))")
    return gameplay_tags


def add_target_tags_component(effect_cdo, tag_names):
    if not tag_names:
        effect_cdo.set_editor_property("ge_components", [])
        return

    unreal.GASLesson2EffectLibrary.configure_granted_tags(
        effect_cdo,
        make_gameplay_tags(tag_names),
    )


def set_scalable_float_property(target, property_name, value):
    scalable_float = target.get_editor_property(property_name)
    scalable_float.set_editor_property("value", float(value))
    target.set_editor_property(property_name, scalable_float)


def configure_effect(
    asset_name,
    duration_policy,
    attribute_name,
    magnitude=None,
    duration=None,
    period=None,
    set_by_caller_tag=None,
    granted_tags=None,
    stacking=False,
):
    blueprint = create_or_load_blueprint(asset_name, EFFECT_PATH, unreal.GameplayEffect.static_class())
    effect_cdo = get_blueprint_cdo(blueprint)
    effect_cdo.modify()

    effect_cdo.set_editor_property("duration_policy", duration_policy)
    effect_cdo.set_editor_property(
        "modifiers",
        [make_modifier(attribute_name, magnitude, set_by_caller_tag)],
    )

    if duration is not None:
        duration_template = effect_cdo.get_editor_property("duration_magnitude")
        effect_cdo.set_editor_property("duration_magnitude", make_scalable_magnitude(duration_template, duration))

    set_scalable_float_property(effect_cdo, "period", period or 0.0)
    effect_cdo.set_editor_property("execute_periodic_effect_on_application", period is not None)

    if stacking:
        effect_cdo.set_editor_property("stacking_type", unreal.GameplayEffectStackingType.AGGREGATE_BY_TARGET)
        effect_cdo.set_editor_property("stack_limit_count", 5)
        effect_cdo.set_editor_property(
            "stack_duration_refresh_policy",
            unreal.GameplayEffectStackingDurationPolicy.REFRESH_ON_SUCCESSFUL_APPLICATION,
        )
    else:
        effect_cdo.set_editor_property("stacking_type", unreal.GameplayEffectStackingType.NONE)
        effect_cdo.set_editor_property("stack_limit_count", 0)

    add_target_tags_component(effect_cdo, granted_tags or [])
    blueprint.modify()
    require(unreal.EditorAssetLibrary.save_loaded_asset(blueprint, False), f"Failed to save {asset_name}")
    log(f"Configured {asset_name}")
    return blueprint


def set_character_effect_defaults(character_blueprint, effects):
    character_cdo = get_blueprint_cdo(character_blueprint)
    character_cdo.modify()

    property_map = {
        "damage_effect": "GE_Damage",
        "heal_effect": "GE_Heal",
        "attack_buff_effect": "GE_AttackBuff",
        "weapon_effect": "GE_Weapon",
        "dynamic_damage_effect": "GE_DynamicDamage",
        "poison_effect": "GE_Poison",
        "stack_attack_effect": "GE_StackAttack",
    }
    for property_name, effect_name in property_map.items():
        character_cdo.set_editor_property(property_name, effects[effect_name].generated_class())

    character_blueprint.modify()
    require(unreal.EditorAssetLibrary.save_loaded_asset(character_blueprint, False), "Failed to save BP_GASLesson2Character")
    log("Bound all GameplayEffect classes on BP_GASLesson2Character")


def set_game_mode_default_pawn(game_mode_blueprint, character_blueprint):
    game_mode_cdo = get_blueprint_cdo(game_mode_blueprint)
    game_mode_cdo.modify()
    game_mode_cdo.set_editor_property("default_pawn_class", character_blueprint.generated_class())
    game_mode_blueprint.modify()
    require(unreal.EditorAssetLibrary.save_loaded_asset(game_mode_blueprint, False), "Failed to save BP_GASLesson2GameMode")
    log("Set BP_GASLesson2Character as the default pawn")


def verify_effect(
    asset_name,
    expected_policy,
    expected_attribute,
    expected_magnitude=None,
    expected_duration=None,
    expected_period=0.0,
    expected_set_by_caller_tag=None,
    expected_granted_tags=None,
    expected_stack_limit=0,
):
    blueprint = unreal.EditorAssetLibrary.load_asset(f"{EFFECT_PATH}/{asset_name}")
    require(blueprint is not None, f"Missing {asset_name}")
    effect_cdo = get_blueprint_cdo(blueprint)
    require(effect_cdo.get_editor_property("duration_policy") == expected_policy, f"{asset_name} duration policy mismatch")
    modifiers = effect_cdo.get_editor_property("modifiers")
    require(len(modifiers) == 1, f"{asset_name} modifier count mismatch")
    require(
        modifiers[0].get_editor_property("attribute").get_editor_property("attribute_name") == expected_attribute,
        f"{asset_name} attribute mismatch",
    )
    modifier_text = modifiers[0].export_text()
    if expected_magnitude is not None:
        require(
            f"Value={float(expected_magnitude):.6f}" in modifier_text,
            f"{asset_name} magnitude mismatch: {modifier_text}",
        )
    if expected_set_by_caller_tag is not None:
        require("MagnitudeCalculationType=SetByCaller" in modifier_text, f"{asset_name} is not SetByCaller")
        require(
            f'TagName="{expected_set_by_caller_tag}"' in modifier_text,
            f"{asset_name} SetByCaller tag mismatch",
        )

    period = effect_cdo.get_editor_property("period").get_editor_property("value")
    require(abs(period - expected_period) < 0.001, f"{asset_name} period mismatch: {period}")
    if expected_duration is not None:
        duration_text = effect_cdo.get_editor_property("duration_magnitude").export_text()
        require(
            f"Value={float(expected_duration):.6f}" in duration_text,
            f"{asset_name} duration mismatch: {duration_text}",
        )

    require(
        effect_cdo.get_editor_property("stack_limit_count") == expected_stack_limit,
        f"{asset_name} stack limit mismatch",
    )
    for tag_name in expected_granted_tags or []:
        tag = unreal.GameplayTag()
        tag.import_text(f'(TagName="{tag_name}")')
        require(
            unreal.GASLesson2EffectLibrary.has_granted_tag(effect_cdo, tag),
            f"{asset_name} missing granted tag {tag_name}",
        )
    log(f"Verified {asset_name}")


def verify_blueprint_defaults(effects):
    character_blueprint = unreal.EditorAssetLibrary.load_asset(f"{ROOT_PATH}/BP_GASLesson2Character")
    game_mode_blueprint = unreal.EditorAssetLibrary.load_asset(f"{ROOT_PATH}/BP_GASLesson2GameMode")
    require(character_blueprint is not None, "Missing BP_GASLesson2Character")
    require(game_mode_blueprint is not None, "Missing BP_GASLesson2GameMode")

    property_map = {
        "damage_effect": "GE_Damage",
        "heal_effect": "GE_Heal",
        "attack_buff_effect": "GE_AttackBuff",
        "weapon_effect": "GE_Weapon",
        "dynamic_damage_effect": "GE_DynamicDamage",
        "poison_effect": "GE_Poison",
        "stack_attack_effect": "GE_StackAttack",
    }
    character_cdo = get_blueprint_cdo(character_blueprint)
    for property_name, effect_name in property_map.items():
        require(
            character_cdo.get_editor_property(property_name) == effects[effect_name].generated_class(),
            f"BP_GASLesson2Character.{property_name} mismatch",
        )

    game_mode_cdo = get_blueprint_cdo(game_mode_blueprint)
    require(
        game_mode_cdo.get_editor_property("default_pawn_class") == character_blueprint.generated_class(),
        "BP_GASLesson2GameMode default pawn mismatch",
    )
    log("Verified Character effect bindings and GameMode default pawn")


def main():
    if not unreal.EditorAssetLibrary.does_directory_exist(EFFECT_PATH):
        require(unreal.EditorAssetLibrary.make_directory(EFFECT_PATH), f"Failed to create {EFFECT_PATH}")

    effects = {
        "GE_Damage": configure_effect(
            "GE_Damage", unreal.GameplayEffectDurationType.INSTANT, "Health", magnitude=-20.0
        ),
        "GE_Heal": configure_effect(
            "GE_Heal", unreal.GameplayEffectDurationType.INSTANT, "Health", magnitude=20.0
        ),
        "GE_AttackBuff": configure_effect(
            "GE_AttackBuff",
            unreal.GameplayEffectDurationType.HAS_DURATION,
            "Attack",
            magnitude=50.0,
            duration=5.0,
            granted_tags=["Demo.Effect", "Demo.Effect.AttackBuff"],
        ),
        "GE_Weapon": configure_effect(
            "GE_Weapon",
            unreal.GameplayEffectDurationType.INFINITE,
            "Attack",
            magnitude=20.0,
            granted_tags=["Demo.Effect", "Demo.Effect.Weapon"],
        ),
        "GE_DynamicDamage": configure_effect(
            "GE_DynamicDamage",
            unreal.GameplayEffectDurationType.INSTANT,
            "Health",
            set_by_caller_tag="Data.Damage",
        ),
        "GE_Poison": configure_effect(
            "GE_Poison",
            unreal.GameplayEffectDurationType.HAS_DURATION,
            "Health",
            magnitude=-5.0,
            duration=10.0,
            period=1.0,
            granted_tags=["Demo.Effect", "Demo.Effect.Poison"],
        ),
        "GE_StackAttack": configure_effect(
            "GE_StackAttack",
            unreal.GameplayEffectDurationType.HAS_DURATION,
            "Attack",
            magnitude=10.0,
            duration=10.0,
            granted_tags=["Demo.Effect", "Demo.Effect.Stack"],
            stacking=True,
        ),
    }

    character_blueprint = create_or_load_blueprint(
        "BP_GASLesson2Character",
        ROOT_PATH,
        unreal.GASLesson2Character.static_class(),
    )
    game_mode_blueprint = create_or_load_blueprint(
        "BP_GASLesson2GameMode",
        ROOT_PATH,
        unreal.GASLesson2GameMode.static_class(),
    )
    set_character_effect_defaults(character_blueprint, effects)
    set_game_mode_default_pawn(game_mode_blueprint, character_blueprint)

    verify_effect("GE_Damage", unreal.GameplayEffectDurationType.INSTANT, "Health", expected_magnitude=-20.0)
    verify_effect("GE_Heal", unreal.GameplayEffectDurationType.INSTANT, "Health", expected_magnitude=20.0)
    verify_effect(
        "GE_AttackBuff",
        unreal.GameplayEffectDurationType.HAS_DURATION,
        "Attack",
        expected_magnitude=50.0,
        expected_duration=5.0,
        expected_granted_tags=["Demo.Effect", "Demo.Effect.AttackBuff"],
    )
    verify_effect(
        "GE_Weapon",
        unreal.GameplayEffectDurationType.INFINITE,
        "Attack",
        expected_magnitude=20.0,
        expected_granted_tags=["Demo.Effect", "Demo.Effect.Weapon"],
    )
    verify_effect(
        "GE_DynamicDamage",
        unreal.GameplayEffectDurationType.INSTANT,
        "Health",
        expected_set_by_caller_tag="Data.Damage",
    )
    verify_effect(
        "GE_Poison",
        unreal.GameplayEffectDurationType.HAS_DURATION,
        "Health",
        expected_magnitude=-5.0,
        expected_duration=10.0,
        expected_period=1.0,
        expected_granted_tags=["Demo.Effect", "Demo.Effect.Poison"],
    )
    verify_effect(
        "GE_StackAttack",
        unreal.GameplayEffectDurationType.HAS_DURATION,
        "Attack",
        expected_magnitude=10.0,
        expected_duration=10.0,
        expected_granted_tags=["Demo.Effect", "Demo.Effect.Stack"],
        expected_stack_limit=5,
    )
    verify_blueprint_defaults(effects)
    log("SUCCESS: all GASLesson2 assets were created and verified")


main()
