//TESH.scrollpos=284
//TESH.alwaysfold=0

/////////////////////////////
// Initialization function //
/////////////////////////////
function InitTrig_Triggered_Incinerate takes nothing returns nothing

    local trigger t1  = CreateTrigger()
    local trigger t2  = CreateTrigger()
    set udg_TI_Hash   = InitHashtable()

    call TriggerRegisterAnyUnitEventBJ( t1, EVENT_PLAYER_UNIT_DEATH )
    call TriggerAddCondition( t1, Filter( function TI_Incinerate ) )
    call TriggerRegisterVariableEvent( t2, "udg_DamageEvent", EQUAL, 1 )
    call TriggerAddCondition( t2, Filter( function TI_MainAction ) )
    call ExecuteFunc( "Trig_Incinerate_SpellsList_Actions" )

endfunction 

////////////////////////////////////////////////////
// This function indexes the spell's stats listed //
// in the Incinerate spells list one by one       //
////////////////////////////////////////////////////
function TI_InitSpellIndex takes nothing returns nothing

    local integer i = udg_TI_SpellIndex + 1

    set udg_TI_SpellID[i] = udg_TI_Spell
    set udg_TI_BuffID[i] = udg_TI_Buff
    set udg_TI_SpellAsTrigger[i] = udg_TI_AllowSpellAsTrigger
    set udg_TI_DPABase[i] = udg_TI_DamageStackBase
    set udg_TI_DPAPerLevel[i] = udg_TI_DamageStackPerLevel
    set udg_TI_DurationBase[i] = udg_TI_DurBase
    set udg_TI_DurationPerLevel[i] = udg_TI_DurPerLevel
    set udg_TI_DamageCapBase[i] = udg_TI_DCapBase
    set udg_TI_DamageCapPerLevel[i] = udg_TI_DCapPerLevel
    set udg_TI_ExplodeAOEBase[i] = udg_TI_EXAOEBase
    set udg_TI_ExplodeAOEPerLevel[i] = udg_TI_EXAOEPerLevel
    set udg_TI_ExplodeSmallDAOEBase[i] = udg_TI_EXSmallAOEBase
    set udg_TI_ExplodeSmallDAOEPerLev[i] = udg_TI_EXSmallAOEPerLevel
    set udg_TI_ExplodeSmallDFactorBase[i] = udg_TI_EXSmallDamageFactorBase
    set udg_TI_ExplodeSmallDFactorPerLev[i] = udg_TI_EXSmallDamageFactorPerLevel
    set udg_TI_AT[i] = udg_TI_AttackType
    set udg_TI_DT[i] = udg_TI_DamageType
    set udg_TI_SFXModel1[i] = udg_TI_EffectModel1
    set udg_TI_SFXModel2[i] = udg_TI_EffectModel2
    set udg_TI_AttachPoint[i] = udg_TI_EffectAttachment
    set udg_TI_IsUnitStructure[i] = udg_TI_Structures
    set udg_TI_IsUnitMechanical[i] = udg_TI_Mechanicals
    set udg_TI_IsUnitMagicImmune[i] = udg_TI_MagicImmune
    set udg_TI_IsUnitAlly[i] = udg_TI_Allies
    set udg_TI_IsUnitIllusion[i] = udg_TI_Illusions

    set udg_TI_SpellIndex = i

endfunction

/////////////////////////////////////////////////////
// This function debuffs the target unit when not  //
// damaged for a certain duration                  //
/////////////////////////////////////////////////////
function TI_Debuff takes nothing returns nothing

    local timer t           = GetExpiredTimer()
    local integer timerkey  = GetHandleId( t )
    local integer i         = LoadInteger( udg_TI_Hash, timerkey, 0 )
    local unit u            = LoadUnitHandle( udg_TI_Hash, timerkey, 1 )
    local integer key       = GetHandleId( u )
    local string I          = I2S( i )

    call UnitRemoveAbilityBJ( udg_TI_BuffID[i], u )
    call DestroyEffect( LoadEffectHandle( udg_TI_Hash, key, StringHash( "SFX" + I ) ) )
    call DestroyTimer( t )
    call SaveBoolean( udg_TI_Hash, key, i, false )
    call SaveReal( udg_TI_Hash, key, StringHash( "Damage" + I ), 0 )
    call FlushChildHashtable( udg_TI_Hash, timerkey )

    set t = null
    set u = null
    set I = null

endfunction


//////////////////////////////////////////////////////
// This function triggers the dead unit's explosion //
//////////////////////////////////////////////////////
function TI_Incinerate takes nothing returns nothing

    local unit dying        = GetTriggerUnit()
    local integer key       = GetHandleId( dying )
    local real x            = GetUnitX( dying )
    local real y            = GetUnitY( dying )
    local integer i         = 0
    local unit target
    local player p
    local real dx
    local real dy
    local real damage
    local real smalldfactor
    local unit buffplacer
    local string I

    set udg_Semafor = false
    call DisplayTextToForce( GetPlayersAll(), "Semafor na false" )
    loop

        set i = i + 1
        set I = I2S( i )

        if LoadBoolean( udg_TI_Hash, key, i ) then

            set damage = LoadReal( udg_TI_Hash, key, StringHash( "AOEDamage" + I ) )
            set smalldfactor = LoadReal( udg_TI_Hash, key, StringHash( "SmallDFactor" + I ) )
            set buffplacer = LoadUnitHandle( udg_TI_Hash, key, StringHash( "BuffPlacer" + I ) )
            set p = GetOwningPlayer( buffplacer )
            call SaveReal( udg_TI_Hash, key, StringHash( "Damage" + I ), 0 )
            call DestroyEffect( LoadEffectHandle( udg_TI_Hash, key, StringHash( "SFX" + I ) ) )
            call DestroyEffect( AddSpecialEffect( udg_TI_SFXModel2[i], x, y ) )
            call GroupEnumUnitsInRange( udg_TI_TempGroup, x, y, LoadReal( udg_TI_Hash, key, StringHash( "SmallDAOE" + I ) ), null )

            loop

                set target = FirstOfGroup( udg_TI_TempGroup )
                exitwhen target == null
                call GroupRemoveUnit( udg_TI_TempGroup, target )
                set dx = GetUnitX( target ) - x
                set dy = GetUnitY( target ) - y

                if ( target != buffplacer ) and TI_TargetFilter( target, p, i ) then
              	    call DisableTrigger( gg_trg_Damage_Engine )
                    if SquareRoot( dx*dx + dy*dy ) > LoadReal( udg_TI_Hash, key, StringHash( "AOE" + I ) ) then

                        call UnitDamageTarget( buffplacer, target, damage*smalldfactor, true, false, udg_TI_AT[i], udg_TI_DT[i], null )

                    else

                        call UnitDamageTarget( buffplacer, target, damage, true, false, udg_TI_AT[i], udg_TI_DT[i], null )

                    endif
		    call EnableTrigger( gg_trg_Damage_Engine )
                endif

            endloop

        endif

        exitwhen i == udg_TI_SpellIndex

    endloop

    call FlushChildHashtable( udg_TI_Hash, key )
    set udg_Semafor = true
    call DisplayTextToForce( GetPlayersAll(), "Semafor na true" )
    set dying = null
    set buffplacer = null
    set p = null
    set I = null

endfunction


///////////////////////////////////////////////////////////
// This function checks if the level of incinerate spell //
// for the damage source is greater than zero in order   //
// run the main actions of the spell.                    //
///////////////////////////////////////////////////////////
function TI_MainAction takes nothing returns nothing
    local integer key    = GetHandleId( udg_DamageEventTarget )
    local integer i      = 0
    local integer level
    local integer timerkey
    local real dcap
    local string I
	
    if udg_Semafor then
        set udg_Semafor = false
        call DisplayTextToForce( GetPlayersAll(), "Semafor na false" )
        loop
            
            set i = i + 1
            set level = GetUnitAbilityLevel( udg_DamageEventSource, udg_TI_SpellID[i] )

            if level > 0 and udg_IsDamageAttack then

                if not udg_IsDamageSpell or udg_TI_SpellAsTrigger[i] then

                    set I = I2S( i )
                    set dcap = udg_TI_DamageCapBase[i] + udg_TI_DamageCapPerLevel[i]*level

                    if TI_TargetFilter( udg_DamageEventTarget, GetOwningPlayer( udg_DamageEventSource ), i ) then
                call UnitAddAbilityBJ( udg_TI_BuffID[i], udg_DamageEventTarget )
                        call DestroyEffect( LoadEffectHandle( udg_TI_Hash, key, StringHash( "SFX" + I ) ) )
                        call SaveEffectHandle( udg_TI_Hash, key, StringHash( "SFX" + I ), AddSpecialEffectTarget( udg_TI_SFXModel1[i], udg_DamageEventTarget, udg_TI_AttachPoint[i] ) )
                        call DestroyTimer( LoadTimerHandle( udg_TI_Hash, key, StringHash( "Timer" + I ) ) )
                        call SaveTimerHandle( udg_TI_Hash, key, StringHash( "Timer" + I ), CreateTimer() )
                        set timerkey = GetHandleId( LoadTimerHandle( udg_TI_Hash, key, StringHash( "Timer" + I ) ) )
                        call SaveUnitHandle( udg_TI_Hash, timerkey, 1, udg_DamageEventTarget )
                        call SaveUnitHandle( udg_TI_Hash, key, StringHash( "BuffPlacer" + I ), udg_DamageEventSource )
                        call SaveReal( udg_TI_Hash, key, StringHash( "Damage" + I ), LoadReal( udg_TI_Hash, key, StringHash( "Damage" + I ) ) + udg_TI_DPABase[i] + udg_TI_DPAPerLevel[i]*level )
                        call SaveReal( udg_TI_Hash, key, StringHash( "AOEDamage" + I ), udg_TI_AOEDamageBase + udg_TI_AOEDamagePerLevel*level )
                	call SaveReal( udg_TI_Hash, key, StringHash( "AOE" + I ), udg_TI_ExplodeAOEBase[i] + udg_TI_ExplodeAOEPerLevel[i]*level )
                        call SaveReal( udg_TI_Hash, key, StringHash( "SmallDAOE" + I ), udg_TI_ExplodeSmallDAOEBase[i] + udg_TI_ExplodeSmallDAOEPerLev[i]*level )
                        call SaveReal( udg_TI_Hash, key, StringHash( "SmallDFactor" + I  ), udg_TI_ExplodeSmallDFactorBase[i] + udg_TI_ExplodeSmallDFactorPerLev[i]*level )
                        call SaveInteger( udg_TI_Hash, timerkey, 0, i )
                        call SaveBoolean( udg_TI_Hash, key, i, true )

                        if dcap != 0 then

                            if LoadReal( udg_TI_Hash, key, StringHash( "Damage" + I ) ) > dcap then

                                call SaveReal( udg_TI_Hash, key, StringHash( "Damage" + I ), dcap )

                            endif

                        endif

                        
                        call UnitDamageTarget( udg_DamageEventSource, udg_DamageEventTarget, udg_DamageEventAmount + LoadReal( udg_TI_Hash, key, StringHash( "Damage" + I ) ), true, false, udg_TI_AT[i], udg_TI_DT[i], null )
                        call DisplayTextToForce( GetPlayersAll(), "Zadałem dmg" )
                        set udg_DamageEventAmount = 0
                        call TimerStart( LoadTimerHandle( udg_TI_Hash, key, StringHash( "Timer" + I ) ), udg_TI_DurationBase[i] + udg_TI_DurationPerLevel[i]*level, false, function TI_Debuff )

                    endif

                endif

            endif

            exitwhen i == udg_TI_SpellIndex

        endloop
    endif
    set udg_Semafor = true
    call DisplayTextToForce( GetPlayersAll(), "Semafor na true" )
    set I = null
endfunction

/////////////////////////////////////////////////////
// This function filters unwanted units from being //
// affected by the spell                           //
/////////////////////////////////////////////////////
function TI_TargetFilter takes unit target, player p, integer i returns boolean
    return ( ( not IsUnitType( target, UNIT_TYPE_STRUCTURE ) ) or udg_TI_IsUnitStructure[i] ) and ( ( not IsUnitType( target, UNIT_TYPE_MECHANICAL ) ) or udg_TI_IsUnitMechanical[i] ) and ( ( not IsUnitType( target, UNIT_TYPE_MAGIC_IMMUNE ) ) or udg_TI_IsUnitMagicImmune[i] ) and ( IsUnitEnemy( target, p ) or udg_TI_IsUnitAlly[i] ) and ( ( not IsUnitIllusion( target ) ) or udg_TI_IsUnitIllusion[i] )
endfunction
