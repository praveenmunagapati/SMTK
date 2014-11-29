<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "BooleanUnion" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="union" BaseType="operator">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="2" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Int Name="keep inputs" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(union)" BaseType="result">
      <ItemDefinitions>
        <!-- The united body (or bodies). -->
        <ModelEntity Name="bodies" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>