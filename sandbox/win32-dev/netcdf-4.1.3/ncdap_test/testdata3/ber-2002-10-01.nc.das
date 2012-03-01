Attributes {
    TSurfAir {
        String standard_name "TSurfAir";
        String long_name "Surface Air Temperature";
        Float32 _FillValue -1.000000000;
        String units "K";
    }
    Delta_Time {
        String long_name "offset to time";
        String Description "Cell observation time, in offsets to values defined in the time variable";
        String units "seconds";
    }
    nv {
        String long_name "number_of_vertices";
    }
    time_bnds {
        String long_name "time_bnds";
        String units "seconds since 1993-1-1 0:0:0";
    }
    time {
        String long_name "time";
        String axis "T";
        String units "seconds since 1993-1-1 0:0:0";
        String bounds "time_bnds";
    }
    latitude {
        String long_name "latitude";
        String units "degrees_north";
        String standard_name "latitude";
        String axis "Y";
        String GCTP_Projection_Name "Geographic";
        String GCTP_Spheroid_Name "Clarke 1866";
    }
    longitude {
        String long_name "longitude";
        String units "degrees_east";
        String standard_name "longitude";
        String axis "X";
        String GCTP_Projection_Name "Geographic";
        String GCTP_Spheroid_Name "Clarke 1866";
    }
    NC_GLOBAL {
        String Conventions "CF-1.0";
        String Institution "CSISS,GMU";
    }
    Lineage {
        String History "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<History xmlns:smXML=\"http://www.isotc211.org/smXML\" 
 xmlns:xlink=\"http://www.w3.org/1999/xlink\" 
 xmlns:gml=\"http://www.opengis.net/gml\">

     <lineage>
        <LI_Lineage>
          <statement>
		 <CharacterString>
               This lineage information documents the source and processing performed by the WCS server in the CEOP satellite data server.
             </CharacterString>
          </statement>
          <processStep>
            <LI_ProcessStep>
              <description>
                 <CharacterString>
	                  <Reprojection>
				    <Name>SatelliteSwathToLatitudeLongitude</Name>
				    <Method>ForwardMapping</Method>
				</Reprojection>
                 </CharacterString>
              </description>
              <rationale>
                <CharacterString>
			 Perform georectification by transforming satellite swath coordinate system to Latitude-Longitude coordinate system.  
                </CharacterString>
              </rationale>
              <dateTime>
                <DateTime>2008-05-21T20:12:12.00000Z</DateTime>
              </dateTime>
              <processor>
                <CI_ResponsibleParty>
                  <individualName>
                    <CharacterString>Wenli Yang</CharacterString>
                  </individualName>
                  <organisationName>
                    <CharacterString>NASA GES DISC</CharacterString>
                  </organisationName>
                  <contactInfo>
                    <CI_Contact>
                      <phone>
                        <CI_Telephone>
                          <voice>
                            <CharacterString>301-614-5312</CharacterString>
                          </voice>
                        </CI_Telephone>
                      </phone>
                          <electronicMailAddress>
                            <CharacterString>Wenli.Yang@nasa.gov</CharacterString>
                          </electronicMailAddress>
                    </CI_Contact>
                  </contactInfo>
                </CI_ResponsibleParty>
              </processor>
          </LI_ProcessStep>
           <LI_ProcessStep>
              <description>
                 <CharacterString>
				<Interpolation>
				    <Name>Interpolation</Name>
				    <Method> NearestNeighbor Interpolation </Method>
				</Interpolation>
                 </CharacterString>
              </description>
              <rationale>
                <CharacterString>Interpolatiion is needed for output cells containing no valid input values.</CharacterString>
              </rationale>
              <dateTime>
                <DateTime>2008-05-21T20:12:12.000000Z</DateTime>
              </dateTime>
              <processor>
                <CI_ResponsibleParty>
                  <individualName>
                    <CharacterString>Wenli Yang</CharacterString>
                  </individualName>
                  <organisationName>
                    <CharacterString>NASA GES DISC</CharacterString>
                  </organisationName>
                  <contactInfo>
                    <CI_Contact>
                      <phone>
                        <CI_Telephone>
                          <voice>
                            <CharacterString>301-614-5312</CharacterString>
                          </voice>
                        </CI_Telephone>
                      </phone>
                          <electronicMailAddress>
                            <CharacterString>Wenli.Yang@nasa.gov</CharacterString>
                          </electronicMailAddress>
                    </CI_Contact>
                  </contactInfo>
                </CI_ResponsibleParty>
              </processor>
          </LI_ProcessStep>
           <LI_ProcessStep>
              <description>
                 <CharacterString>
				<QualityScreen>
				    <Name>QualityScreen</Name>
				    <Method>AIRSPerPixelQAFiltering</Method>
				</QualityScreen>
                 </CharacterString>
              </description>
              <rationale>
                <CharacterString>Input cells not meeting QA criteria must be excluded in the resultant coverage.</CharacterString>
              </rationale>
              <dateTime>
                <DateTime>2008-05-21T20:12:12.000000Z</DateTime>
              </dateTime>
              <processor>
                <CI_ResponsibleParty>
                  <individualName>
                    <CharacterString>Wenli Yang</CharacterString>
                  </individualName>
                  <organisationName>
                    <CharacterString>NASA GES DISC</CharacterString>
                  </organisationName>
                  <contactInfo>
                    <CI_Contact>
                      <phone>
                        <CI_Telephone>
                          <voice>
                            <CharacterString>301-614-5312</CharacterString>
                          </voice>
                        </CI_Telephone>
                      </phone>
                          <electronicMailAddress>
                            <CharacterString>Wenli.Yang@nasa.gov</CharacterString>
                          </electronicMailAddress>
                    </CI_Contact>
                  </contactInfo>
                </CI_ResponsibleParty>
              </processor>
          </LI_ProcessStep>
         </processStep>


          <source>
              <LI_Source>
                  <description>
                    <CharacterString>Input AIRS Release 5 Level 2 granule</CharacterString>
                  </description>
                  <sourceReferenceSystem>
                    <MD_ReferenceSystem>
                      <referenceSystemIdentifier>
                        <RS_Identifier>
					<code>
						<CharacterString>OGC:Swath</CharacterString>
					</code>
					<codeSpace>
						<CharacterString>OGC</CharacterString>
					</codeSpace>
				</RS_Identifier>
			   </referenceSystemIdentifier>
		       </MD_ReferenceSystem>
                  </sourceReferenceSystem>
                  <sourceCitation>
                    <CI_Citation>
                      <title>
                        <CharacterString>Input AIRS Release 5 Level 2 granule</CharacterString>
                      </title>
                      <date>
                        <CI_Date>
                          <date>
                            <Date>2004-10-01</Date>
                          </date>
                          <dateType>
                            <CI_DateTypeCode>publication</CI_DateTypeCode>
                          </dateType>
                        </CI_Date>
                      </date>
                      <identifier>
                        <MD_Identifier>
                          <authority />
                          <code>
                            <CharacterString />
                          </code>
                        </MD_Identifier>
                      </identifier>
                      <citedResponsibleParty>
                        <CI_ResponsibleParty>
                          <organisationName>
                            <CharacterString>NASA GES DISC</CharacterString>
                          </organisationName>
                          <contactInfo>
                            <CI_Contact>
                              <onlineResource>
                                <CI_OnlineResource>
                                  <linkage>
                                    <URL>ftp://airspar1u.ecs.nasa.gov/data/s4pa/Aqua_AIRS_Level2/AIRX2RET.005/2002/274/AIRS.2002.10.01.097.L2.RetStd.v5.0.14.0.G07227014224.hdf</URL>
                                  </linkage>
                                  <protocol>
                                    <CharacterString>ftp</CharacterString>
                                  </protocol>
                                  <function>
                                    <CI_OnLineFunctionCode>download</CI_OnLineFunctionCode>
                                  </function>
                                </CI_OnlineResource>
                              </onlineResource>
                            </CI_Contact>
                          </contactInfo>
                          <role>
                            <CI_RoleCode>originator</CI_RoleCode>
                          </role>
                        </CI_ResponsibleParty>
                      </citedResponsibleParty>
                      <presentationForm>
                        <CI_PresentationFormCode codeSpace=\"\" codeList=\"\" codeListValue=\"\" />
                      </presentationForm>
                    </CI_Citation>
                  </sourceCitation>
                  <sourceExtent>
                    <EX_Extent>
                      <description>
                        <CharacterString />
                      </description>
				<geographicElement>
					<EX_BoundingPolygon>
						<polygon>
							<gml:Polygon srsName=\"urn:EPSG:geographicCRS:4326\">
								<gml:outerBoundaryIs>
									<gml:LinearRing>
										<gml:pos>59.542054 -91.525519</gml:pos>
										<gml:pos>64.006578 -122.761953</gml:pos>
										<gml:pos>42.866636 -126.317500</gml:pos>
										<gml:pos>40.097498 -106.352680</gml:pos>
									</gml:LinearRing>
								</gml:outerBoundaryIs>
							</gml:Polygon>
						</polygon>
					</EX_BoundingPolygon>
				</geographicElement>
                       <temporalElement>
					<EX_TemporalExtent>
						<extent>
							<gml:TimePeriod>
								<gml:begin>
									<gml:TimeInstant>
										<gml:timePosition>2002-10-01T09:18:00.000000Z</gml:timePosition>
									</gml:TimeInstant>
								</gml:begin>
								<gml:end>
									<gml:TimeInstant>
										<gml:timePosition>2002-10-01T09:23:59.999999Z</gml:timePosition>
									</gml:TimeInstant>
								</gml:end>
							</gml:TimePeriod>
						</extent>
					</EX_TemporalExtent>
				</temporalElement>
                    </EX_Extent>
                  </sourceExtent>
              </LI_Source>
              <LI_Source>
                  <description>
                    <CharacterString>Input AIRS Release 5 Level 2 granule</CharacterString>
                  </description>
                  <sourceReferenceSystem>
                    <MD_ReferenceSystem>
                      <referenceSystemIdentifier>
                        <RS_Identifier>
					<code>
						<CharacterString>OGC:Swath</CharacterString>
					</code>
					<codeSpace>
						<CharacterString>OGC</CharacterString>
					</codeSpace>
				</RS_Identifier>
			   </referenceSystemIdentifier>
		       </MD_ReferenceSystem>
                  </sourceReferenceSystem>
                  <sourceCitation>
                    <CI_Citation>
                      <title>
                        <CharacterString>Input AIRS Release 5 Level 2 granule</CharacterString>
                      </title>
                      <date>
                        <CI_Date>
                          <date>
                            <Date>2004-10-01</Date>
                          </date>
                          <dateType>
                            <CI_DateTypeCode>publication</CI_DateTypeCode>
                          </dateType>
                        </CI_Date>
                      </date>
                      <identifier>
                        <MD_Identifier>
                          <authority />
                          <code>
                            <CharacterString />
                          </code>
                        </MD_Identifier>
                      </identifier>
                      <citedResponsibleParty>
                        <CI_ResponsibleParty>
                          <organisationName>
                            <CharacterString>NASA GES DISC</CharacterString>
                          </organisationName>
                          <contactInfo>
                            <CI_Contact>
                              <onlineResource>
                                <CI_OnlineResource>
                                  <linkage>
                                    <URL>ftp://airspar1u.ecs.nasa.gov/data/s4pa/Aqua_AIRS_Level2/AIRX2RET.005/2002/274/AIRS.2002.10.01.193.L2.RetStd.v5.0.14.0.G07227072611.hdf</URL>
                                  </linkage>
                                  <protocol>
                                    <CharacterString>ftp</CharacterString>
                                  </protocol>
                                  <function>
                                    <CI_OnLineFunctionCode>download</CI_OnLineFunctionCode>
                                  </function>
                                </CI_OnlineResource>
                              </onlineResource>
                            </CI_Contact>
                          </contactInfo>
                          <role>
                            <CI_RoleCode>originator</CI_RoleCode>
                          </role>
                        </CI_ResponsibleParty>
                      </citedResponsibleParty>
                      <presentationForm>
                        <CI_PresentationFormCode codeSpace=\"\" codeList=\"\" codeListValue=\"\" />
                      </presentationForm>
                    </CI_Citation>
                  </sourceCitation>
                  <sourceExtent>
                    <EX_Extent>
                      <description>
                        <CharacterString />
                      </description>
				<geographicElement>
					<EX_BoundingPolygon>
						<polygon>
							<gml:Polygon srsName=\"urn:EPSG:geographicCRS:4326\">
								<gml:outerBoundaryIs>
									<gml:LinearRing>
										<gml:pos>50.182469 -108.424849</gml:pos>
										<gml:pos>53.793611 -84.244362</gml:pos>
										<gml:pos>74.889234 -88.293173</gml:pos>
										<gml:pos>68.007927 -132.994761</gml:pos>
									</gml:LinearRing>
								</gml:outerBoundaryIs>
							</gml:Polygon>
						</polygon>
					</EX_BoundingPolygon>
				</geographicElement>
                       <temporalElement>
					<EX_TemporalExtent>
						<extent>
							<gml:TimePeriod>
								<gml:begin>
									<gml:TimeInstant>
										<gml:timePosition>2002-10-01T19:46:00.000000Z</gml:timePosition>
									</gml:TimeInstant>
								</gml:begin>
								<gml:end>
									<gml:TimeInstant>
										<gml:timePosition>2002-10-01T19:51:59.999999Z</gml:timePosition>
									</gml:TimeInstant>
								</gml:end>
							</gml:TimePeriod>
						</extent>
					</EX_TemporalExtent>
				</temporalElement>
                    </EX_Extent>
                  </sourceExtent>
              </LI_Source>
          </source>
        </LI_Lineage>
      </lineage>
</History> ";
    }
}
