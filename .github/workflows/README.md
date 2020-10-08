Debug github actions.

Add the following step before the failing step.
ssh into the tmate session immediately. 
Otherwise the session times out and doesn't accept any input.

```
    - name: Setup tmate session
      uses: mxschmitt/action-tmate@v2
```

