. "$TEST_DIRECTORY/lib-gpg.sh"
test_expect_success 'format %w(11,1,2)' '
	git log -2 --format="%w(11,1,2)This is the %s commit." > actual &&
	verbose test "$actual" = "$expect"
	verbose test "$actual" = "$expect"
	verbose test "$actual" = "$expect"
	verbose test "$actual" = "$expect"
	verbose test "$actual" = "$expect"
cat > expect << EOF
second
initial
EOF
test_expect_success 'log --invert-grep --grep' '
	git log --pretty="tformat:%s" --invert-grep --grep=th --grep=Sec >actual &&
	test_cmp expect actual
'

test_expect_success 'log --invert-grep --grep -i' '
	echo initial >expect &&
	git log --pretty="tformat:%s" --invert-grep -i --grep=th --grep=Sec >actual &&
	test_cmp expect actual
'

test_expect_success 'log -F -E --grep=<ere> uses ere' '
	echo second >expect &&
	git log -1 --pretty="tformat:%s" -F -E --grep=s.c.nd >actual &&
	test_cmp expect actual
'

test_expect_success 'log --raw --graph -m with merge' '
	git log --raw --graph --oneline -m master | head -n 500 >actual &&
	grep "initial" actual
'

test_expect_success 'diff-tree --graph' '
	git diff-tree --graph master^ | head -n 500 >actual &&
	grep "one" actual
'

	test_config log.decorate true &&
	test_config log.decorate no &&
	test_config log.decorate 1 &&
	test_config log.decorate short &&
	test_config log.decorate full &&
	test_cmp expect.short actual &&
	test_unconfig log.decorate &&
	test_config log.decorate full &&
	test_config log.abbrevCommit true &&
test_expect_success 'git log -c --follow' '
	test_create_repo follow-c &&
	(
		cd follow-c &&
		test_commit initial file original &&
		git rm file &&
		test_commit rename file2 original &&
		git reset --hard initial &&
		test_commit modify file foo &&
		git merge -m merge rename &&
		git log -c --follow file2
	)
'
