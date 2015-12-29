# For installation of public include files.
install(FILES
  brahms-1065.h brahms-1065_hdr.h brahms-1065_impl.h  
  brahms-1199.h brahms-1199_hdr.h brahms-1199_impl.h  
  brahms-1266.h
  brahms-client.h brahms-component.h
  brahms-c++-common.h brahms-c++-legacy.h
  DESTINATION ${INCLUDE_INSTALL_PATH})

install(FILES brahms.xml DESTINATION ${SHARE_BRAHMS_INSTALL_PATH})
if(LICENSE_INSTALL)
  install(FILES LICENSE DESTINATION ${SHARE_BRAHMS_INSTALL_PATH})
endif()
