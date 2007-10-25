<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >
  <xsl:import href="utility.xsl"/>
  <xsl:output indent="yes" method="xml" doctype-system="xtupleapi.dtd" />

  <xsl:template match="/">
    <xsl:element name="xtupleimport">
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>

  <xsl:template name="YahooCountry">
    <xsl:param name="country"/>
    SELECT country_name FROM country WHERE (country_abbr=
    <xsl:choose>
      <xsl:when test="substring($country,1,2) = 'GB'">'UK'</xsl:when>
      <xsl:otherwise>'<xsl:value-of select="substring($country,1,2)"/>'</xsl:otherwise>
    </xsl:choose>)
  </xsl:template>

  <xsl:template name="customerNumberFromAddress">
    <xsl:param name="address"/>
      <xsl:choose>
	<xsl:when test="$address/Company">
	  getCustNumberForCompany('<xsl:value-of select="$address/Company"/>')
	</xsl:when>
	<xsl:when test="$address/Name/First
		    and $address/Name/Last">
	  getCustNumberForName('<xsl:value-of select="$address/Name/First"/>',
			       '<xsl:value-of select="$address/Name/Last"/>')
	</xsl:when>
	<xsl:otherwise>
	  getCustNumberForName('<xsl:value-of select="$address/Name/Full"/>')
	</xsl:otherwise>
      </xsl:choose>
  </xsl:template>

  <xsl:template match="OrderList">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="Order">
    <salesorder>
      <order_number>
	<xsl:call-template name="stripAlpha">
	  <xsl:with-param name="number" select="@id"/>
	</xsl:call-template>
      </order_number>

      <!-- warehouse -->

      <order_date>
	<xsl:value-of select="Time"/>
      </order_date>

      <!-- pack_date -->

      <originated_by value="Internet"/>

      <!-- sales_rep -->
      <!-- commission -->
      <!-- tax_authority -->
      <!-- terms -->
      <!-- project_number -->

      <customer_number>
	<xsl:choose>
	  <xsl:when test="AddressInfo[@type = 'bill']">
	    <xsl:call-template name="customerNumberFromAddress">
	      <xsl:with-param name="address"
			      select="AddressInfo[@type = 'bill']"/>
	    </xsl:call-template>
	  </xsl:when>
	  <xsl:when test="AddressInfo[@type = 'ship']">
	    <xsl:call-template name="customerNumberFromAddress">
	      <xsl:with-param name="address"
			      select="AddressInfo[@type = 'ship']"/>
	    </xsl:call-template>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:message terminate="yes">
	      Cannot find Customer Number for Order
	      <xsl:value-of select="number"/>
	    </xsl:message>
	  </xsl:otherwise>
	</xsl:choose>
      </customer_number>

      <xsl:if test="AddressInfo[@type = 'bill']">
	<xsl:if test="AddressInfo[@type = 'bill']/Name/Full">
	  <billto_name>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Name/Full"/>
	  </billto_name>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Address1">
	  <billto_address1>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Address1"/>
	  </billto_address1>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Address2">
	  <billto_address2>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Address2"/>
	  </billto_address2>
	</xsl:if>
	<!-- billto_address3 -->
	<xsl:if test="AddressInfo[@type = 'bill']/City">
	  <billto_city>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/City"/>
	  </billto_city>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/State">
	  <billto_state>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/State"/>
	  </billto_state>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Zip">
	  <billto_postal_code>
	    <xsl:value-of select="AddressInfo[@type = 'bill']/Zip"/>
	  </billto_postal_code>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'bill']/Country">
	  <billto_country>
	    <xsl:call-template name="YahooCountry">
	      <xsl:with-param name="country"
			      select="AddressInfo[@type='bill']/Country"/>
	    </xsl:call-template>
	  </billto_country>
	</xsl:if>
      </xsl:if>

      <xsl:if test="AddressInfo[@type = 'ship']">
	<!-- shipto_number -->
	<xsl:if test="AddressInfo[@type = 'ship']/Name/Full">
	  <shipto_name>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/Name/Full"/>
	  </shipto_name>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/Address1">
	  <shipto_address1>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/Address1"/>
	  </shipto_address1>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/Address2">
	  <shipto_address2>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/Address2"/>
	  </shipto_address2>
	</xsl:if>
	<!-- shipto_address3 -->
	<xsl:if test="AddressInfo[@type = 'ship']/City">
	  <shipto_city>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/City"/>
	  </shipto_city>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/State">
	  <shipto_state>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/State"/>
	  </shipto_state>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/Zip">
	  <shipto_postal_code>
	    <xsl:value-of select="AddressInfo[@type = 'ship']/Zip"/>
	  </shipto_postal_code>
	</xsl:if>
	<xsl:if test="AddressInfo[@type = 'ship']/Country">
	  <shipto_country>
	    <xsl:call-template name="YahooCountry">
	      <xsl:with-param name="country"
			      select="AddressInfo[@type='ship']/Country"/>
	    </xsl:call-template>
	  </shipto_country>
	</xsl:if>
      </xsl:if>

      <!-- TODO: how does Yahoo encode Discover, MasterCard, and AmEx?
	   Once we know the answer we might be able to figure out how
	   to set cust_po_number
        -->
      <!-- cust_po_number-->

      <!-- fob -->

      <xsl:if test="Shipping">
	<ship_via>
	  <xsl:value-of select="Shipping"/>
	</ship_via>
      </xsl:if>

      <!-- hold_type -->

      <!-- shipping_chgs -->
      <!-- shipping_form -->
      <!-- ship_complete -->

      <currency>
	<xsl:value-of select="@currency"/>
      </currency>

      <!-- misc_charge_description -->
      <!-- misc_account_number -->
      <!-- misc_charge -->
      <!-- freight -->

      <order_notes>
	<xsl:if test="Comments">
	  <xsl:value-of select="Comments"/>
	  <xsl:text>
</xsl:text>
	</xsl:if>
	<xsl:if test="CreditCard">
	  <xsl:value-of select="CreditCard"/>
	  type <xsl:value-of select="CreditCard/@type"/>
	  expiration <xsl:value-of select="CreditCard/@expiration"/>
	  <xsl:text>
</xsl:text>
	</xsl:if>
	<xsl:for-each select="AddressInfo">
	  <xsl:value-of select="@type"/>: 
	  <xsl:for-each select="Phone|Email">
	    <xsl:value-of select="name()"/>= <xsl:value-of select="text()"/>
	    <xsl:text> </xsl:text>
	  </xsl:for-each>
	  <xsl:text>
</xsl:text>
	</xsl:for-each>
	<xsl:for-each select="AddressInfo/Custom">
	  <xsl:value-of select="concat(name(), '-', @name)"/>: <xsl:value-of select="text()"/>
	  <xsl:text>
</xsl:text>
	</xsl:for-each>
	<xsl:if test="Coupon">
	  Coupon: <xsl:value-of select="Coupon/*"/>
	  <xsl:text>
</xsl:text>
	</xsl:if>
	<xsl:for-each select="NumericTime		|
			      Referer			|
			      Entry-Point		|
			      IPAddress			|
			      YahooLogin		|
			      Trackable-Link		|
			      Yahoo-Shopping-Order	|
			      Space-Id			|
			      Bogus			|
			      OSBS			|
			      Warning			|
			      Suspect			|
			      Store-Status		|
			      HTTP-User-Agent		|
			      GiftWrap			|
			      GiftWrapMessage		|
			      CardEvents">
	  <xsl:sort select="name()"/>
	  <xsl:value-of select="name()"/>: <xsl:value-of select="text()"/>
	  <xsl:text>
</xsl:text>
	</xsl:for-each>
	<xsl:if test="Cookie">
	  Cookie B=<xsl:value-of select="Cookie/@B"/>
		 Y=<xsl:value-of select="Cookie/@Y"/>
	  <xsl:text>
</xsl:text>
	</xsl:if>
      </order_notes>

      <!-- shipping_notes -->
      <!-- add_to_packing_list_batch -->

    </salesorder>
    <xsl:apply-templates select="Item"/>
  </xsl:template>

  <xsl:template match="Item">
    <salesline>
      <order_number>
	<xsl:call-template name="stripAlpha">
	  <xsl:with-param name="number" select="../@id"/>
	</xsl:call-template>
      </order_number>

      <line_number>
	<xsl:value-of select="@num"/>
      </line_number>

      <item_number>
	<xsl:value-of select="Id"/>
      </item_number>

      <!-- customer_pn -->
      <!-- substitute_for -->
      <!-- sold_from_whs -->

      <qty_ordered>
	<xsl:value-of select="Quantity"/>
      </qty_ordered>

      <net_unit_price>
	<xsl:value-of select="Unit-Price"/>
      </net_unit_price>

      <!-- scheduled_date -->
      <!-- tax_code -->
      <!-- discount_pct_from_list -->
      <!-- create_order -->
      <!-- overwrite_po_price -->

      <notes>
	<!-- ignore Thumb -->
	<xsl:for-each select="Code		|
			      Description		|
			      Url			|
			      Taxable">
	  <xsl:sort select="name()"/>
	  <xsl:value-of select="name()"/>: <xsl:value-of select="text()"/>
	  <xsl:text>
  </xsl:text>
	</xsl:for-each>
      </notes>
    </salesline>

    <xsl:apply-templates select="Option|OptionLists"/>
  </xsl:template>

  <xsl:template name="saleslinechar">
    <xsl:param name="id"/>
    <xsl:param name="line-number"/>
    <xsl:param name="characteristic"/>
    <xsl:param name="value"/>
    <saleslinechar>
      <order_number>
	<xsl:call-template name="stripAlpha">
	  <xsl:with-param name="number" select="$id"/>
	</xsl:call-template>
      </order_number>

      <line_number>
	<xsl:value-of select="$line-number"/>
      </line_number>

      <characteristic>
	<xsl:value-of select="$characteristic"/>
      </characteristic>

      <value>
	<xsl:value-of select="$value"/>
      </value>
    </saleslinechar>
  </xsl:template>

  <xsl:template match="Option">
    <xsl:call-template name="saleslinechar">
      <xsl:with-param name="id"			select="../../@id"	/>
      <xsl:with-param name="line-number"	select="../@num"	/>
      <xsl:with-param name="characteristic"	select="@name"		/>
      <xsl:with-param name="value"		select="text()"		/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="OptionLists">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="OptionList">
    <xsl:variable name="characteristic" select="@name"/>
    <xsl:for-each select="OptionValue">
      <xsl:call-template name="saleslinechar">
	<xsl:with-param name="id"		select="../../../@id"	 />
	<xsl:with-param name="line-number"	select="../../@num"	 />
	<xsl:with-param name="characteristic"	select="$characteristic" />
	<xsl:with-param name="value"		select="text()"		 />
      </xsl:call-template>
    </xsl:for-each>
  </xsl:template>

</xsl:stylesheet>
