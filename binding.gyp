{
  "targets": [
    {
      "target_name": "<(module_name)",
      "sources": [
        "src/psapi.cc",
        "src/enumerate-processes.cc",
        "src/enumerate-modules.cc"
      ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "defines":["UNICODE","_UNICODE"]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
