extract_and_move_object
Arguments:
	- object_name: Name of the structure, class, or object to extract
	- source_file: Source file containing the original definition

Execute the instructions one at a time.
1. Read the contents of <source_file> to isolate the definition of <object_name>. Use the bash command `sed -n "<start-line>,<end-line>p" <source_file>` to print the object interval lines.
2. Answer the questions below:
    - What is the hierarchy of this object (namespace)?
    - Is this object header only, or does it also need the cpp file?
    - What includes are needed in this new file?
    - What using statements are needed in this new file?
3. Execute cpp_create_class with the answers from the previous question.
4. If the object is a niebloid, go to step `7`.
5. If the object is a class, transfer the code to new file.
6. Goto to step `8`.
7. Remove the class automatically created by the template and transfer the code to the new file.
8. Use the `cpp_code_verifier` tool on the newly created files. If the tool reports any [warning-onlu] violations, make the requested adjustments.
9. Clean up the legacy code from the original file <source_file>.
10. Run the `cpp_compile` tool. If there are any errors, do not adjust or investigate them; simply report them to the user.

