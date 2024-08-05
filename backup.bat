del _.zip
"C:\Program Files\7-Zip\7z.exe" a -tzip _.zip -mx9 "*.bat" "*.txt" "*.dsp" "*.dsw" "*.cpp" "*.h" "*.str" "diff_bin"
if %errorlevel% neq 0 pause
