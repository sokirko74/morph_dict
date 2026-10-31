# slf_convert

Converts paradigms in the slf format (Tools/Paradigm Export of MorphWizard) to a dictionary of MorphWizard and back,
without the GUI.

```
slf_convert --to-mrd --slf paradigms.slf --mwz folder/project.mwz [--user name]
slf_convert --to-slf --mwz folder/project.mwz --slf paradigms.slf
```
`--to-mrd` adds all paradigms of the slf file to the dictionary of the project (an absent mrd file is created empty),
the gramtab (`gramtab.json`) is read from the folder of the project. Paradigms with errors are reported with the line
number and skipped. A user other than `guest` must be listed in `USERS` of `project.mwz`.
