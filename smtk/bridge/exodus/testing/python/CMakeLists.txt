set(smtkExodusSessionPythonTests
  # Empty for now.
)

# Additional tests that require SMTK_DATA_DIR
set(smtkExodusSessionPythonDataTests
  exoReadFile
  slacReadFile
  smtkReadFile
)

foreach (test ${smtkExodusSessionPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkExodusSessionPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()
