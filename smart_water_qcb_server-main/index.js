const cors = require('cors');
const express = require('express');
const mongoose = require('mongoose');

const UserModel = require('./modles/user');

const app = express();
app.use(express.json());
app.use(cors());

mongoose.connect('mongodb+srv://sri:sri123@sepjt.km4mib2.mongodb.net/');



// User Signup
app.post('/signup', (req, res) => {
  const { email, password } = req.body;

    UserModel.findOne({ email: req.body.email }).then((userByEmail) => {
      if (userByEmail) {
        res.json('Email already registered');
        return;
      }

      UserModel.create(req.body).then((user) => {
        res.json(user);
      }).catch(err => res.json(err));
    });
  });

// User Login
app.post('/login', (req, res) => {
  const { email, password } = req.body;

  UserModel.findOne({ email }).then((user) => {
    if (!user) {
      // User not found
      res.status(200).json({message : 'User not found'});
      return;
    }

    if (user.password === password) {
      // Login successful
      res.json('Success');
    } else {
      // Incorrect password
      res.status(200).json({message : 'Wrong password'});
    }
  });
});


app.listen(process.env.PORT || 5000, () => {
  console.log('Server started on port 3000');
});