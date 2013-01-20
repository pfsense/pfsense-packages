pfsense-packages
================

Snort Package for pfSense 2.x

Package Version: 2.5.3
Snort Binary Version: 2.9.2.3

----------------------------------------------------------------
Change Log: 01/19/2013
----------------------------------------------------------------
New Features:

      1.  Ability to use Snort VRT pre-defined IPS policies
          and the associated rules by selecting a policy.
          The pre-defined polices are Connectivity, Balanced
          or Security.

      2.  Automatic flowbit resolution for enabled rules. This
          ensures required "flowbit-set" rules are enabled to
          satisfy any "flowbit-isset" checks in selected rules.

      3.  An additional setting for configuring memcap on the
          stream5 preprocessor.

      4.  Added configuration for two new SCADA industrial
          controls preprocessors for Modbus and DNP3.  This
          allows use of SCADA rules from Digital Bond, Snort
          VRT and Emerging Threats.

      5.  Added color-coded flagging of modified SID rules
          in the GUI Rules display tab to make it easy to 
          identify rules whose enabled or disabled state 
          has been modified by the user.

      6.  Added capability to reset modified rules to their
          default state and remove any enablesid/disablesid 
          mods.  This can be done on a specific category or 
          to all categories.

      6.  Added option to disable alerts from the http_inspect
          preprocessor without actually disabling its function.

Bug Fixes:

      1.  Corrected inconsistencies in the behavior of 
          enablesid/disablesid changes across rule updates.
          User customizations of rule enable/disable states
          now persist across automated rule updates and 
          Snort start/stop cycles.

      2.  Corrected problems related to potentially building
          incorrect sid-msg.map, classification.config and
          reference.config files during automated rule updates.

      3.  Fixed bug in handling of zero values for some
          http_inspect parameters.  Zeroes were evaluting as
          empty variables and the desired value was not 
          getting written to the snort.conf file.

      4.  Fixed assorted minor HTML layout issues with some
          of the Snort GUI tab screens.

      5.  Added code to the automated rules update routine
          so that multiple attempts at rules download are 
          made before logging an error and giving up.  This
          helps during times the rule download web sites are
          particularly busy.
