import { useState } from "react"
import Navbar from "./components/Navbar"
import FoodCard from "./components/FoodCard"
import Cart from "./components/Cart"
import FoodDetails from "./components/FoodDetails"


const foods = [
  {
    id: 1,
    name: "Chicken Burger",
    description: "Delicious chicken burger with fresh ingredients.",
    price: 120,
    category: "Burger",
    image: "🍔",
  },
  {
    id: 2,
    name: "Veg Pizza",
    description: "Fresh vegetable pizza with delicious toppings.",
    price: 180,
    category: "Pizza",
    image: "🍕",
  },
  {
    id: 3,
    name: "Italian Pasta",
    description: "Creamy pasta prepared with fresh ingredients.",
    price: 150,
    category: "Pasta",
    image: "🍝",
  },
  {
    id: 4,
    name: "French Fries",
    description: "Crispy golden fries served hot and fresh.",
    price: 80,
    category: "Snacks",
    image: "🍟",
  },
  {
    id: 5,
    name: "Cold Coffee",
    description: "Refreshing cold coffee prepared with milk.",
    price: 70,
    category: "Drinks",
    image: "🥤",
  },
]

const categories = [
  "All",
  "Burger",
  "Pizza",
  "Pasta",
  "Snacks",
  "Drinks",
]

function App() {
  const [cart, setCart] = useState([])
  const [selectedFood, setSelectedFood] = useState(null)
const [favorites, setFavorites] = useState([])

  // Search and category state
  const [searchTerm, setSearchTerm] = useState("")
  const [selectedCategory, setSelectedCategory] = useState("All")

  const handleAddToCart = (food) => {
    setCart((currentCart) => {
      const existingItem = currentCart.find(
        (item) => item.id === food.id
      )

      if (existingItem) {
        return currentCart.map((item) =>
          item.id === food.id
            ? { ...item, quantity: item.quantity + 1 }
            : item
        )
      }

      return [
        ...currentCart,
        {
          ...food,
          quantity: 1,
        },
      ]
    })
  }

  const handleUpdateQuantity = (foodId, newQuantity) => {
    if (newQuantity <= 0) {
      handleRemoveItem(foodId)
      return
    }

    setCart((currentCart) =>
      currentCart.map((item) =>
        item.id === foodId
          ? { ...item, quantity: newQuantity }
          : item
      )
    )
  }

  const handleRemoveItem = (foodId) => {
    setCart((currentCart) =>
      currentCart.filter((item) => item.id !== foodId)
    )
  }

  const handleToggleFavorite = (foodId) => {
  setFavorites((currentFavorites) => {
    if (currentFavorites.includes(foodId)) {
      return currentFavorites.filter((id) => id !== foodId)
    }

    return [...currentFavorites, foodId]
  })
}

  const cartItemCount = cart.reduce(
    (total, item) => total + item.quantity,
    0
  )

  // Filter food using search text and selected category
  const filteredFoods = foods.filter((food) => {
    const matchesSearch = food.name
      .toLowerCase()
      .includes(searchTerm.toLowerCase())

    const matchesCategory =
      selectedCategory === "All" ||
      food.category === selectedCategory

    return matchesSearch && matchesCategory
  })

  return (
    <div className="min-h-screen bg-gray-100">

      <Navbar cartItemCount={cartItemCount} />

      {/* Hero */}
      <section className="bg-orange-50 py-20">

        <div className="max-w-7xl mx-auto px-6 text-center">

          <h1 className="text-5xl font-bold text-gray-800 mb-6">
            Delicious Food, Delivered Fast
          </h1>

          <p className="text-lg text-gray-600 mb-8">
            Order your favourite food from the comfort of your home.
          </p>

          {/* Search */}
          <div className="max-w-xl mx-auto">

            <input
              type="text"
              value={searchTerm}
              onChange={(event) => setSearchTerm(event.target.value)}
              placeholder="Search for food..."
              className="w-full px-5 py-4 rounded-xl border border-gray-300
                         focus:outline-none focus:ring-2 focus:ring-orange-500"
            />

          </div>

        </div>

      </section>

      {/* Categories */}
      <section className="max-w-7xl mx-auto px-6 py-10">

        <h2 className="text-3xl font-bold text-gray-800 mb-6">
          Categories
        </h2>

        <div className="flex flex-wrap gap-4">

          {categories.map((category) => (

            <button
              key={category}
              onClick={() => setSelectedCategory(category)}
              className={
                selectedCategory === category
                  ? "bg-orange-600 text-white px-6 py-3 rounded-full"
                  : "bg-white px-6 py-3 rounded-full shadow hover:bg-orange-50"
              }
            >
              {category}
            </button>

          ))}

        </div>

      </section>

      {/* Food */}
      <section className="max-w-7xl mx-auto px-6 pb-16">

        <h2 className="text-3xl font-bold text-gray-800 mb-8">
          Popular Food
        </h2>

        {filteredFoods.length === 0 ? (

          <div className="bg-white rounded-2xl shadow-md p-10 text-center">

            <div className="text-5xl mb-4">
              🔍
            </div>

            <h3 className="text-xl font-bold text-gray-800">
              No food found
            </h3>

            <p className="text-gray-500 mt-2">
              Try another search or category.
            </p>

          </div>

        ) : (

          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-8">

            {filteredFoods.map((food) => (
              <FoodCard
  key={food.id}
  food={food}
  onAddToCart={handleAddToCart}
  onViewDetails={() => setSelectedFood(food)}
/>
            ))}

          </div>

        )}

      </section>

      {/* Cart */}
      <section className="max-w-4xl mx-auto px-6 pb-20">

        <Cart
          cart={cart}
          onUpdateQuantity={handleUpdateQuantity}
          onRemoveItem={handleRemoveItem}
        />

      </section>

          {/* Food Details Modal */}
      <FoodDetails
        food={selectedFood}
        onClose={() => setSelectedFood(null)}
        onAddToCart={handleAddToCart}
        isFavorite={
          selectedFood
            ? favorites.includes(selectedFood.id)
            : false
        }
        onToggleFavorite={handleToggleFavorite}
      />

    </div>
  )
}

export default App